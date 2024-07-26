
#include "FS.h"
#include "SdFat.h"
#include "sdios.h"
#include "mimetable.h"
// #include <ArduinoJson.h>

static const char TEXT_PLAIN[] PROGMEM = "text/plain";
static const char FS_INIT_ERROR[] PROGMEM = "FS INIT ERROR";
static const char FILE_NOT_FOUND[] PROGMEM = "FileNotFound";

// JsonDocument doc;
String currentDir;
fs::File uploadFile;

bool canHandle(AsyncWebServerRequest *request) {
  //request->addInterestingHeader("ANY");
  return true;
}
String captiveList[23] = { "clients3.google.com",
                           "clients4.google.com",
                           "android.clients.google.com",
                           "connectivitycheck.android.com",
                           "connectivitycheck.gstatic.com",
                           "www.gstatic.com",
                           "www.google.com",
                           "www.androidbak.net",
                           "www.apple.com",
                           "www.appleiphonecell.com",
                           "captive.apple.com",
                           "www.airport.us",
                           "www.ibook.info",
                           "www.itools.info",
                           "www.thinkdifferent.us",
                           "apple.com",
                           "captive.apple.com",
                           "http://connectivitycheck.android.com/generate_204",
                           "http://connectivitycheck.gstatic.com/generate_204",
                           "d2uzsrnmmf6tds.cloudfront.net",
                           "clients3.google.com",
                           "msftncsi.com",
                           "/uma/v2" };


class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}


  void handleRequest(AsyncWebServerRequest *request) {

    String thisUrl = "" + String(request->url());
    for (int i = 0; i < 23; i++) {
      if (thisUrl.endsWith(captiveList[i])) {
        request->redirect(localIPURL + mainPath + "/index");
        return;
      }
    }
    bool redirectCondition;
    redirectCondition = thisUrl.endsWith(mainPath + "/ncsi.txt") || thisUrl.endsWith(mainPath + "/hotspot-detect.html") || thisUrl.endsWith(mainPath + "/canonical.html") || thisUrl.endsWith(mainPath + "/success.txt") || thisUrl.endsWith(mainPath + "/connecttest.txt") || thisUrl.endsWith(mainPath + "/wpad.dat") || thisUrl.endsWith(mainPath + "/generate_204");
    redirectCondition |= thisUrl.endsWith(mainPath + "/chrome-variations/seed");

    if (redirectCondition) {
      request->redirect(localIPURL + mainPath + "/index");
      return;
    }



    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
    response->print("<p>This is out captive portal front page.</p>");
    response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
    response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
    response->print("</body></html>");
    request->send(response);
    request->redirect(localIPURL + mainPath + "/index");
  }
};


////////////////////////////////
// Utils to return HTTP codes, and determine content-type

void replyOK(AsyncWebServerRequest *request) {
  request->send(200, FPSTR(TEXT_PLAIN), "");
}

void replyOKWithMsg(AsyncWebServerRequest *request, String msg) {

  request->send(200, FPSTR(TEXT_PLAIN), msg);
}

void replyNotFound(AsyncWebServerRequest *request, String msg) {
  request->send(404, FPSTR(TEXT_PLAIN), msg);
}

void replyBadRequest(AsyncWebServerRequest *request, String msg) {
  DBG_OUTPUT_PORT.println(msg);
  request->send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void replyServerError(AsyncWebServerRequest *request, String msg) {
  DBG_OUTPUT_PORT.println(msg);
  request->send(500, FPSTR(TEXT_PLAIN), msg + "\r\n");
}



// **********File operations  ******************

////////////////////////////////
// Request handlers
// list all of the files, if ishtml=true, return html rather than simple text
String listFiles(String path) {
  String returnText = "{";
  Serial.println("Listing files stored on SD");
  File root = SD.open(path);
  File foundfile = root.openNextFile();

  do {
    if (foundfile.isDirectory()) {  //if is a folder
      returnText += "{";
      returnText += "name:'" + String(foundfile.name()) + "',";
      returnText += "size:'" + String(foundfile.size()) + "',";
      returnText += "type: 1 ,";  // 0 for file, 1 for folder , 2 for raw, 3 for unknown
      returnText += "path:'" + String(foundfile.path()) + "', ";
      returnText += "last_mod_time:'" + String(foundfile.getLastWrite()) + "' ";
      returnText += "}";
    } else {  //if is a directory
      returnText += "{";
      returnText += "name:'" + String(foundfile.name()) + "',";
      returnText += "size:'" + String(foundfile.size()) + "',";
      returnText += "type: 0 ,";  // 0 for file, 1 for folder , 2 for raw, 3 for unknown
      returnText += "path:'" + String(foundfile.path()) + "', ";
      returnText += "last_mod_time:'" + String(foundfile.getLastWrite()) + "' ";
      returnText += "}";
    }
    foundfile = root.openNextFile();
    if (foundfile) {
      returnText += ",";
    }
  } while (foundfile);
  returnText += "}";
  root.close();
  foundfile.close();
  return returnText;
}



// /*
//    Return the list of files in the directory specified by the "dir" query string parameter.
//    Also demonstrates the use of chunked responses.
// */
void handleFileList(AsyncWebServerRequest *request) {
  machineState = BUSSY;
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }
  if (!SD.exists(mainPath + "/")) { return replyServerError(request, FPSTR(FS_INIT_ERROR)); }

  if (!request->hasArg("comm1")) { return replyBadRequest(request, F("DIR ARG MISSING")); }

  String path = request->arg("comm1");
  if (path != mainPath + "/" && !SD.exists(path)) { return replyBadRequest(request, "BAD PATH"); }
  DBG_OUTPUT_PORT.println(String("handleFileList: ") + path);
  request->send(200, "application/json", listFiles(path));
  handleFileList(request);
}

/*
   Return the FS type, status and size info
*/
void handleStatus(AsyncWebServerRequest *request) {
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }

  DBG_OUTPUT_PORT.println("handleStatus");
  String json;
  json.reserve(128);

  json = "{\"type\":\"";
  json += "SDFS";
  json += "\", \"isOk\":";
  if (SD.exists(mainPath + "/")) {
    json += F("\"true\", \"totalBytes\":\"");
    json += SD.totalBytes();
    json += F("\", \"usedBytes\":\"");
    json += SD.usedBytes();
    json += "\"";
  } else {
    json += "\"false\"";
  }
  json += F(",\"unsupportedFiles\":\"");
  json += unsupportedFiles;
  json += "\"}";

  request->send(200, "application/json", json);
  handleFileList(request);
}

void gZipCb(AsyncWebServerRequest *request) {
  AsyncWebServerResponse *response;
  response->addHeader("Content-Encoding", "gzip");
  request->send(200);
}

/*
   Read the given file from the filesystem and stream it back to the client
*/
bool handleFileRead(AsyncWebServerRequest *request, FS *filesys, String path, bool download = true) {
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }

  machineState = BUSSY;
  DBG_OUTPUT_PORT.println(String("handleFileRead: ") + path);

  if (path.endsWith(mainPath + "/")) { return false; }

  String contentType;
  if (download) {
    contentType = F("application/octet-stream");
  } else {
    contentType = getContentType(path);
    DBG_OUTPUT_PORT.println(contentType);
  }
  if (!filesys->exists(path)) {
    // File not found, try gzip version
    path = path + ".gz";
  }

  if (filesys->exists(path)) {
    contentType = F("application/octet-stream");
    machineState = VERY_BUSSY;
    File file = filesys->open(path, "r");
    request->send(*filesys, path, contentType);
    file.close();
    machineState = SUCCESS;
    handleFileList(request);
    return true;
  }
  machineState = ERROR;
  handleFileList(request);
  return false;
}


// /*
//    As some FS (e.g. LittleFS) delete the parent folder when the last child has been removed,
//    return the path of the closest parent still existing
// */
String lastExistingParent(String path) {

  while (!path.isEmpty() && !SD.exists(path)) {
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
void handleFileCreate(AsyncWebServerRequest *request) {
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }
  machineState = BUSSY;
  if (!SD.exists(mainPath + "/")) { return replyServerError(request, FPSTR(FS_INIT_ERROR)); }

  String path = request->arg("comm1");
  if (path.isEmpty()) { return replyBadRequest(request, F("PATH ARG MISSING")); }

#ifdef USE_SPIFFS
  if (checkForUnsupportedPath(path).length() > 0) { return replyServerError(request, F("INVALID FILENAME")); }
#endif

  if (path == mainPath + "/") { return replyBadRequest(request, "BAD PATH"); }
  if (SD.exists(path)) { return replyBadRequest(request, F("PATH FILE EXISTS")); }

  String src = request->arg("comm2");
  if (src.isEmpty() || src == " ") {
    // No source specified: creation
    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path);
    if (path.endsWith(mainPath + "/")) {
      // Create a folder
      path.remove(path.length() - 1);
      if (!SD.mkdir(path)) {
        machineState = ERROR;
        return replyServerError(request, F("MKDIR FAILED"));
      }
    } else {
      // Create a file
      File file = SD.open(path, "w");
      if (file) {
        // file.write((const char*)0);
        Serial.println("File created successfully");
      } else {
        machineState = ERROR;
        return replyServerError(request, F("CREATE FAILED"));
      }
      file.close();
    }
    if (path.lastIndexOf('/') > -1) { path = path.substring(0, path.lastIndexOf('/')); }
    replyOKWithMsg(request, path);
    machineState = SUCCESS;
  } else {
    // Source specified: rename
    if (src == mainPath + "/") {
      machineState = ERROR;
      return replyBadRequest(request, "BAD SRC");
    }
    if (!SD.exists(src)) { return replyBadRequest(request, F("SRC FILE NOT FOUND")); }

    DBG_OUTPUT_PORT.println(String("handleFileCreate: ") + path + " from " + src);

    if (path.endsWith(mainPath + "/")) { path.remove(path.length() - 1); }
    if (src.endsWith(mainPath + "/")) { src.remove(src.length() - 1); }
    if (!SD.rename(src, path)) {
      machineState = ERROR;
      return replyServerError(request, F("RENAME FAILED"));
    }
    replyOK(request);
  }
  handleFileList(request);
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
  File file = SD.open(path, "r");
  bool isDir = file.isDirectory();
  file.close();

  // If it's a plain file, delete it
  if (!isDir) {
    SD.remove(path);
    return;
  }
  // Otherwise delete its contents first
  if (isDir) {
    while (file.openNextFile()) {
      deleteRecursive(path + '/');
    }
    machineState = SUCCESS;
  }
  // Then delete the folder itself
  SD.rmdir(path);
  // Open root directory
  if (!SD.open(mainPath + "/")) {
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
void handleFileDelete(AsyncWebServerRequest *request) {
  machineState = BUSSY;
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }
  if (!SD.exists(mainPath + "/")) {
    machineState = ERROR;
    return replyServerError(request, FPSTR(FS_INIT_ERROR));
  }

  String path = request->arg("comm1");
  if (path.isEmpty() || path == mainPath + "/") { return replyBadRequest(request, "BAD PATH"); }

  DBG_OUTPUT_PORT.println(String("handleFileDelete: ") + path);
  if (!SD.exists(path)) {
    machineState = UNSOLVED;
    return replyNotFound(request, FPSTR(FILE_NOT_FOUND));
  }
  deleteRecursive(path);

  replyOK(request);
}

/*
   Handle a file upload request
*/
void handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = "/shared";
  } else {
    mainPath = "/";
  }
  // make sure authenticated before allowing upload
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }
  if (true) {
    String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
    Serial.println(logmessage);

    if (!index) {
      logmessage = "Upload Start: " + String(filename);
      // open the file on first call and store the file handle in the request object
      request->_tempFile = SPIFFS.open(mainPath + "/" + filename, "w");
      Serial.println(logmessage);
    }

    if (len) {
      // stream the incoming chunk to the opened file
      request->_tempFile.write(*data);
      logmessage = "Writing file: " + String(filename) + " index=" + String(index) + " len=" + String(len);
      Serial.println(logmessage);
    }

    if (final) {
      logmessage = "Upload Complete: " + String(filename) + ",size: " + String(index + len);
      // close the file handle as the upload is now done
      request->_tempFile.close();
      Serial.println(logmessage);
      request->redirect(mainPath + "/");
    }
  } else {
    Serial.println("Auth: Failed");
    return request->requestAuthentication();
  }

  handleFileList(request);
}


/*
   The "Not Found" handler catches all URI not explicitly declared in code
   First try to find and return the requested file from the filesystem,
   and if it fails, return a 404 page with debug information
*/
void handleNotFound(AsyncWebServerRequest *request) {
  machineState = ERROR;
  if (!request->authenticate(webUser, webPass)) {
    // return request->requestAuthentication();
    mainPath = mainPath + "/shared";
  } else {
    mainPath = mainPath + "/";
  }
  if (!SD.exists(mainPath + "/")) { return replyServerError(request, FPSTR(FS_INIT_ERROR)); }
  String logmessage = "Client:" + request->client()->remoteIP().toString() + " " + request->url();
  Serial.println(logmessage);

  if (SD.exists(mainPath + "/fileNotFound.txt")) {
    handleFileRead(request, &SD, mainPath + "/fileNotFound.txt", false);
    machineState = UNSOLVED;
  } else {
    request->send(404, "text/plain", "Not found");
    machineState = UNSOLVED;
  }
  // // Dump debug data
  String message = "Error: File not found\n\nURI:";
  message += request->url();
  message += F("\nMethod: ");
  message += (request->method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += request->args();
  message += '\n';
  for (uint8_t i = 0; i < request->args(); i++) {
    message += F(" NAME:");
    message += request->argName(i);
    message += F("\n VALUE:");
    message += request->arg(i);
    message += '\n';
  }
  message += "path=";
  message += request->arg("path");
  message += '\n';
  DBG_OUTPUT_PORT.print(message);

  // return replyNotFound(request, message);
}

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/
// void handleGetEdit() {
//   if (handleFileRead(F(mainPath + "/edit/index.htm"))) { return; }

// #ifdef INCLUDE_FALLBACK_INDEX_HTM
//   server.header(F("Content-Encoding"), "gzip");
//   server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
// #else
//   replyNotFound(request,FPSTR(FILE_NOT_FOUND));
// #endif
// }
//for admins only

void handleIndexAdmin(AsyncWebServerRequest *request) {
  String logmessage = "Client:" + request->client()->remoteIP().toString() + +" " + request->url();

  if (!request->authenticate(webUser, webPass)) {
    return request->requestAuthentication();
  } else {
    machineState = BUSSY;
    DBG_OUTPUT_PORT.println("handleIndex");
    // Check if index.html exists in the specified path
    if (!SD.exists(mainPath + "/server/index.html")) {
      return replyNotFound(request, "index.html not found");
    }
    // Open the file for reading
    File file = SD.open(mainPath + "/server/index.html", "r");
    if (!file) {
      return replyServerError(request, "Failed to open index.html");
    }

    // Read the content of index.html into a String
    String content;
    while (file.available()) {
      content += (char)file.read();
    }
    file.close();

    // Set the Content-Type header explicitly to text/html
    request->send(200, "text/html", content);

    machineState = SUCCESS;
  }
}


void handleIndex(AsyncWebServerRequest *request) {
  machineState = BUSSY;
  DBG_OUTPUT_PORT.println("handleIndex");

  // Check if index.html exists in the specified path
  if (!SD.exists(mainPath + "/server/index.html")) {
    return replyNotFound(request, "index.html not found");
  }

  // Open the file for reading
  File file = SD.open(mainPath + "/server/index.html", "r");
  if (!file) {
    return replyServerError(request, "Failed to open index.html");
  }

  // Read the content of index.html into a String
  String content;
  while (file.available()) {
    content += (char)file.read();
  }
  file.close();

  // Set the Content-Type header explicitly to text/html
  request->send(200, "text/html", content);

  machineState = SUCCESS;
}


// function handler for handling when user wants to change file name
// handleFileRename(AsyncWebServerRequest *request)

// void handleIcons(){


//   DBG_OUTPUT_PORT.println("handleIndex");
//   String iconName = server.arg("iconName");
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

//   server.header(F("Content-Encoding"), "text");
//   server.send(200, "text/svg", icon);
// }

/*
   This specific handler returns the index.htm (or a gzipped version) from the /edit folder.
   If the file is not present but the flag INCLUDE_FALLBACK_INDEX_HTM has been set, falls back to the version
   embedded in the program code.
   Otherwise, fails with a 404 page with debug information
*/

// void handleGetEdit() {
//   if (handleFileRead(F(mainPath + "/edit/index.htm"))) { return; }

// #ifdef INCLUDE_FALLBACK_INDEX_HTM
//   server.header(F("Content-Encoding"), "gzip");
//   server.send(200, "text/html", index_htm_gz, index_htm_gz_len);
// #else
//   replyNotFound(request,FPSTR(FILE_NOT_FOUND));
// #endif
// }
