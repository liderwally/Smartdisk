#include "FS.h"
#include "SdFat.h"
#include "sdios.h"
#include <ArduinoJson.h>

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

JsonDocument doc;
String currentDir;



////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK() {
  Server.send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(String msg) {
  Server.send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(String msg) {
  Server.send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(String msg) {
  DBG_OUTPUT_PORT.println(msg);
  Server.send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(String msg) {
  DBG_OUTPUT_PORT.println(msg);
  Server.send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}



// **********File operations  ******************

////////////////////////////////
// Request handlers

/*
   Return the FS type, status and size info
*/
void handleStatus() {
  DBG_OUTPUT_PORT.println("handleStatus");
  FSInfo fs_info;
  String json;
  json.reserve(128);

  json = "{\"type\":\"";
  json += fsName;
  json += "\", \"isOk\":";
  if (fsOK) {
    fileSystem->info(fs_info);
    json += F("\"true\", \"totalBytes\":\"");
    json +=  "800000";
    json += F("\", \"usedBytes\":\"");
    json += fs_info.usedBytes;
    json += "\"";
  } else {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  Server.send(200, "application/json", json);
}


// /*
//    Return the list of files in the directory specified by the "dir" query string parameter.
//    Also demonstrates the use of chunked responses.
// */
void handleFileList() {
  machineState = BUSSY ;
  if (!fsOK) { return replyServerError(FPSTR(FS_INIT_ERROR)); }

  if (!Server.hasArg("dir")) { return replyBadRequest(F("DIR ARG MISSING")); }

  String path = Server.arg("dir");
  if (path != "/" && !sdVol->exists(path)) { return replyBadRequest("BAD PATH"); }

  DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
  FsFile file = sdVol->open(path, 0x00);

  if (!dir.open("/")) {
    DBG_OUTPUT_PORT.println("dir.open failed");
  }
  // Open next file in root.
  // Warning, openNext starts at the current position of dir so a
  // rewind may be necessary in your application.
  while (file.openNext(&dir, O_RDONLY)) {
    file.printFileSize(&Serial);
    Serial.write(' ');
    file.printModifyDateTime(&Serial);
    Serial.write(' ');
    file.printName(&Serial);

    Serial.println();
    char *thisfilename ;
    // uint64_t thisfilesize = file.fileSize();
 
    file.getName(thisfilename, 64);
    
    Serial.println(*thisfilename);


    if (file.isDir()) {
      // Indicate a directory.
      Serial.write('/');
    }
    Serial.println();
    file.close();
  }
  if (dir.getError()) {
    Serial.println("openNext failed");
  } else {
    Serial.println("Done!");
  }

  // Server.sendContent('');
  Server.chunkedResponseFinalize();
}


/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(String path) {
  machineState = BUSSY ;
  DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);

  if (path.endsWith("/")) { path += "index.htm"; }

  String contentType;
  if (Server.hasArg("download")) {
    contentType = F("application/octet-stream");
  } else {
    contentType = mime::getContentType(path);
  }

  if (!sdVol->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }
  if (sdVol->exists(path)) {
    machineState = VERY_BUSSY ;
    FsBaseFile file = sdVol->open(path, O_RDWR);
    contentType = mime::getContentType(path);
    if (Server.chunkedResponseModeStart(200, contentType)){ 
      Serial.println();
      Serial.print("fileSize:");
      Serial.println(file.fileSize());
      Serial.print("total of ");
      Serial.print((int)(file.fileSize() ));
      Serial.print(" packets");
      Serial.println();


    for(int j = 0; j<= (int)(file.fileSize() ) ; j++){
      Serial.flush();
      
      char content =file.read();
      Serial.print(content);
      Server.sendContent();
      
    }
    Serial.println();

    Server.chunkedResponseFinalize();
    }
    // if (Server.streamFile(file, contentType) != file.size()) { DBG_OUTPUT_PORT.println("Sent less data than expected!"); }
    file.close();
    machineState = SUCCESS ;
    return true;
    
  }
  machineState = ERROR ;
  return false;
}


// /*
//    As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
//    return the path of the closest parent still existing
// */
String lastExistingParent(String path) {
  while (!path.isEmpty() && !sdVol->exists(path)) {
    if (path.lastIndexOf('/') > 0) {
      path = path.substring(0, path.lastIndexOf('/'));
    } else {
      path = String();  // No slash => the top folder does not exist
    }
  }
  DBG_OUTPUT_PORT.println(String("Last existing parent: ") + path);
  return path;
}

// /*
//    Handle the creation/rename of a new file
//    Operation      | req.responseText
//    ---------------+--------------------------------------------------------------
//    Create file    | parent of created file
//    Create folder  | parent of created folder
//    Rename file    | parent of source file
//    Move file      | parent of source file, or remaining ancestor
//    Rename folder  | parent of source folder
//    Move folder    | parent of source folder, or remaining ancestor
// */
void handleFileCreate() {
  machineState = BUSSY ;
  if (!fsOK) { return replyServerError(FPSTR(FS_INIT_ERROR)); }

  String path = Server.arg("path");
  if (path.isEmpty()) { return replyBadRequest(F("PATH ARG MISSING")); }

#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0) { return replyServerError(F("INVALID FILENAME")); }
#endif

  if (path == "/") { return replyBadRequest("BAD PATH"); }
  if (sdVol->exists(path)) { return replyBadRequest(F("PATH FILE EXISTS")); }

  String src = Server.arg("src");
  if (src.isEmpty()) {
    // No source specified: creation
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith("/")) {
      // Create a folder
      path.remove(path.length() - 1);
      if (!sdVol->mkdir(path)) { 
        machineState = ERROR ;
        return replyServerError(F("MKDIR FAILED")); }
    } else {
      // Create a file
      FsFile file = sdVol->open(path, 0x10);
      if (file) {
      //file.write((const char*)0);
      Serial.println("File created successfully");
      } else {
        machineState = ERROR;
        return replyServerError(F("CREATE FAILED"));
      }
      file.close();
    }
    if (path.lastIndexOf('/') > -1) { path = path.substring(0, path.lastIndexOf('/')); }
    replyOKWithMsg(path);
    machineState = SUCCESS ;
  } else {
    // Source specified: rename
    if (src == "/") { 
      machineState = ERROR ;
      return replyBadRequest("BAD SRC"); }
    if (!sdVol->exists(src)) { return replyBadRequest(F("SRC FILE NOT FOUND")); }

    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith("/")) { path.remove(path.length() - 1); }
    if (src.endsWith("/")) { src.remove(src.length() - 1); }
    if (!sdVol->rename(src, path)) { 
      machineState = ERROR;
      return replyServerError(F("RENAME FAILED")); 
      }
    replyOK();
  }
}


/*
   Delete the file or folder designed by the given path.
   If it's a file, delete it.
   If it's a folder, delete all nested contents first then the folder itself

   IMPORTANT NOTE: using recursion is generally not recommended on embedded devices and can lead to crashes (stack overflow errors).
   This use is just for demonstration purpose, and FSBrowser might crash in case of deeply nested filesystems.
   Please don't do this on a production system.
*/
void deleteRecursive(String path) {
  machineState = VERY_BUSSY;
  FsFile file = sdVol->open(path, O_RDONLY);
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir) {
    sdVol->remove(path);
    return;
  }
  // Otherwise delete its contents first
  if(isDir){
    while (file.openNext(&dir, O_RDONLY)) { 
      // dir.
      // deleteRecursive(path + '/' + ); 
      }
    machineState = SUCCESS ;
  }
  // Then delete the folder itself
  sdVol->rmdir(path);
    // Open root directory
  if (!dir.open("/")) {
    machineState = ERROR;
    DBG_OUTPUT_PORT.println("dir.open failed");
  }
}


/*
   Handle a file deletion request
   Operation      | req.responseText
   ---------------+--------------------------------------------------------------
   Delete file    | parent of deleted file, or remaining ancestor
   Delete folder  | parent of deleted folder, or remaining ancestor
*/
void handleFileDelete() {
  machineState = BUSSY ;
  if (!fsOK) { 
    machineState = ERROR ;
    return replyServerError(FPSTR(FS_INIT_ERROR)); 
    }

  String path = Server.arg(0);
  if (path.isEmpty() || path == "/") { return replyBadRequest("BAD PATH"); }

  DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
  if (!sdVol->exists(path)) { 
    machineState = UNSOLVED;
    return replyNotFound(FPSTR(FILE_NOT_FOUND)); }
  deleteRecursive(path);

  replyOK();
}

/*
   Handle a file upload request
*/
void handleFileUpload() {
  machineState = VERY_BUSSY ;
  if (!fsOK) { 
    machineState = ERROR ;
    return replyServerError(FPSTR(FS_INIT_ERROR)); 
    }
  if (Server.uri() != "/edit") { return; }
  HTTPUpload& upload = Server.upload();
  
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    // Make sure paths always start with "/"
    if (!filename.startsWith("/")) { filename = "/" + filename; }
    DBG_OUTPUT_PORT.println(String("handleFileUpload Name: ") + filename);
    uploadFile = fileSystem->open(filename, "w");
    if (!uploadFile) { return replyServerError(F("CREATE FAILED")); }
    DBG_OUTPUT_PORT.println(String("Upload: START, filename: ") + filename);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      size_t bytesWritten = uploadFile.write(upload.buf, upload.currentSize);
      if (bytesWritten != upload.currentSize) { 
        machineState = ERROR;
        return replyServerError(F("WRITE FAILED")); }
    }
    DBG_OUTPUT_PORT.println(String("Upload: WRITE, Bytes: ") + upload.currentSize);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (uploadFile) { uploadFile.close(); }
    DBG_OUTPUT_PORT.println(String("Upload: END, Size: ") + upload.totalSize);
  }
}


/*
   The "Not Found" handler catches all URI not explicitly declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound() {
  machineState = ERROR;
  if (!fsOK) { return replyServerError(FPSTR(FS_INIT_ERROR)); }

  String uri = ESP8266WebServer::urlDecode(Server.uri());  // required to read paths with blanks

  if (handleFileRead(uri)) { return; }

  // Dump debug data
  String message;
  message.reserve(100);
  message = F("Error: File not found\n\nURI: ");
  message += uri;
  message += F("\nMethod: ");
  message += (Server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += Server.args();
  message += '\n';
  for (uint8_t i = 0; i < Server.args(); i++) {
    message += F(" NAME:");
    message += Server.argName(i);
    message += F("\n VALUE:");
    message += Server.arg(i);
    message += '\n';
  }
  message += "path=";
  message += Server.arg("path");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);
  machineState = UNSOLVED ;
  return replyNotFound(message);
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
// void handleGetEdit() {
//   if (handleFileRead(F("/edit/index.htm"))) { return; }

// #ifdef INCLUDE_FALLBACK_INDEX_HTM
//   Server.sendHeader(F("Content-Encoding"), "gzip");
//   Server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
// #else
//   replyNotFound(FPSTR(FILE_NOT_FOUND));
// #endif
// }

void handleIndex() {
machineState = BUSSY ;
if (handleFileRead(F("/server/index.html"))) { 
  return; 
}else{
  DBG_OUTPUT_PORT.println("handleIndex");
  Server.sendHeader(F("Content-Encoding"), "text");
  Server.send(200, "text/html", indexHtml); 
}
}

// void handleIcons(){


//   DBG_OUTPUT_PORT.println("handleIndex");
//   String iconName = Server.arg("iconName");
//   // String icon = "";
  
//   // bool iconPresent = false;
//   // int iconIndex;

//   // for(int i = 0; i < 120; i++){
//   //   if(iconList[i] == iconName){
//   //     iconPresent = true;
//   //     iconIndex = i;
//   //     break;
//   //   } else {
//   //     iconIndex= -1;
//   //   }
//   // }
//   // if(iconPresent){
//   //   icon = *(IconName);
//   // }



//   String icon = "<?xml version='1.0' encoding='utf-8'?>";

//   Server.sendHeader(F("Content-Encoding"), "text");
//   Server.send(200, "text/svg", icon);
// }

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/

// void handleGetEdit() {
//   if (handleFileRead(F("/edit/index.htm"))) { return; }

// #ifdef INCLUDE_FALLBACK_INDEX_HTM
//   Server.sendHeader(F("Content-Encoding"), "gzip");
//   Server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
// #else
//   replyNotFound(FPSTR(FILE_NOT_FOUND));
// #endif
// }
