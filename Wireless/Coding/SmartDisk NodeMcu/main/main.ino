#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define DBG_OUTPUT_PORT Serial
// defining machine states
#define IDLE        0x01
#define BUSSY       0x02
#define VERY_BUSSY  0x03
#define ERROR       0x04
#define UNSOLVED    0x05
#define SUCCESS     0x06

/* variables for storing machine states
we use memory to make sure the changes are made only 
if machine state is different. To make reduce reccursive checking 
of machine state
*/
size_t machineState = IDLE ;  // for storing current state
size_t lastMachineState = 0x00;  //memory for storing last state 


#ifdef ARDUINO_MAIN
#include <Arduino_FreeRTOS.h>
#include <freertos/task.h>
#endif

uint64_t period = 1000;
float duty = 0.66 ;




#include "SPI.h"
// #include "SdFat.h"
// #include "Adafruit_SPIFlash.h"

// #include "USBMSC.h"

// extern "C" {
// #include <./smb2.h>
// #include "./libsmb2.h"
// }

// #include "html.h"
// #include "sdCard.h"

static bool fsOK;
ESP8266WebServer Server(80);


// #define USE_SPIFFS
#define USE_LITTLEFS
// #define USE_SDFS


#ifdef INCLUDE_FALLBACK_INDEX_HTM
#include "extras/index_htm.h"
#endif



#if defined USE_SPIFFS
#include <FS.h>
const char* fsName = "SPIFFS";
FS* fileSystem = &SPIFFS;
SPIFFSConfig fileSystemConfig = SPIFFSConfig();

#elif defined USE_LITTLEFS
#include <LittleFS.h>
const char* fsName = "LittleFS";
FS* fileSystem = &LittleFS;
LittleFSConfig fileSystemConfig = LittleFSConfig();

#elif defined USE_SDFS
#include <FS.h>
#include <SdFat.h>
#include <SDFS.h>
const char* fsName = "SdFs";
FS* fileSystem = &SDFS;
SDFSConfig fileSystemConfig = SDFSConfig();
// fileSystemConfig.setCSPin(chipSelectPin);
#else
#error Please select a filesystem first by uncommenting one of the "#define USE_xxx" lines at the beginning of the sketch.
#endif





#define STASSID "SmartDisk"
#define STAPSK "walidi"

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "SmartDisk";



String unsupportedFiles;

File uploadFile;



#include "webComponents.h"
#include "icons.h"

#include "sdHelper.h"
FsFile dir;
FsFile file;

#include "severHelper.h"




//*******************************************************
// ***********setup **************************
//**********************************************************
void setup() {
////////////////////////////////
// SERIAL INIT
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print('\n');

////////////////////////////////
///// FILESYSTEM INIT

  //SD Card initiate
  if(!initiateSDCard()){
    cout << F("Failed to initiate the SD card file system") << endl;
  };


  FsFile file = sdVol->open("/myDataLogger.txt", 0x01);
  file.close();
  sdVol->chvol();
  

  // fileSystemConfig.setAutoFormat(false);
  // fileSystem->setConfig(fileSystemConfig);
  fsOK = fileSystem->begin();
  DBG_OUTPUT_PORT.println(fsOK ? F("Filesystem initialized.") : F("Filesystem init failed!"));
  



////////////////////////////////
 // WI-FI INIT
  

  DBG_OUTPUT_PORT.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  
  IPAddress myIP = WiFi.softAPIP();
  DBG_OUTPUT_PORT.print("AP IP address: ");
  DBG_OUTPUT_PORT.println(myIP);

 ////////////////////////////////
 // MDNS INIT
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
    DBG_OUTPUT_PORT.print(F("Open http://"));
    DBG_OUTPUT_PORT.print(host);
    DBG_OUTPUT_PORT.println(F(".local/index to open the FileSystem Browser"));
  }

  ////////////////////////////////
  // WEB SERVER INIT
  // Server.serveStatic("/index.htm", LittleFS, "/server/index.html");
  // server initial request
  Server.on("/index", HTTP_GET, handleIndex);

  // // // server icon request
  // // Server.on("/icons", HTTP_GET, handleIcons

  // // Filesystem status
  Server.on("/status", HTTP_GET, handleStatus);

  // // List directory
  Server.on("/list", HTTP_GET, handleFileList);
Server.
  // // // Load editor
  // // Server.on("/edit", HTTP_GET, handleGetEdit);

  // Create file
  Server.on("/add", HTTP_GET, handleFileCreate);

  // // // Delete file
  Server.on("/delete", HTTP_DELETE, handleFileDelete);

  // // // Upload file
  // // // - first callback is called after the request has ended with all parsed arguments
  // // // - second callback handles file upload at that location
  Server.on("/upload", HTTP_POST, replyOK, handleFileUpload);

  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  Server.onNotFound(handleNotFound);

  // Start server
  if(sdVol->chdir() && sdVol->ls(&DBG_OUTPUT_PORT)){
    DBG_OUTPUT_PORT.println("printing file lists:");
    sdVol->ls();
    file = sdVol->open("/", 0x80);
    FsFile myflie = sdVol->open("/index.html" , 0x10);
    myflie.close();

    Server.begin();
    
    DBG_OUTPUT_PORT.println("HTTP server started");

  }
  // Open root directory
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
  // Open root directory
  if ( sdVol->chdir("/server/") ) {
    DBG_OUTPUT_PORT.println("/server/  folder is present");
    file = sdVol->open("/server/index.html", 0x00);
    // Serial.println(file.readString());
    uint64_t filesize = file.fileSize();
    
    Serial.println("the file size of this folder is :");
    Serial.println(filesize);
  }
  pinMode(2, OUTPUT);
}


void loop() {
  Server.handleClient();
  Serial.flush();
  MDNS.update();
  if(millis() < 1000){
    Serial.println(ESP.getFreeHeap(),DEC);
    fileSystem->gc();
    sdVol->chdir();
    machineState = IDLE;
  }
// defining machine state
  if(machineState != lastMachineState){
    switch(machineState){
      case BUSSY: period= 2000;duty= 0.66;break;
      case VERY_BUSSY: period = 2000;duty= 0.33;break;
      case ERROR : period = 500; duty = 0.66;break;
      case SUCCESS : period = 1000; duty = 0.33;break;
      default: period = 1000;duty = 0.5;break;

    }
    lastMachineState = machineState; 
  }
// indicating machine state
 float deRem = millis()% period;
 if(deRem > 100 && deRem < (duty * period + 100)){
    digitalWrite(2, 1);
  }else{
    digitalWrite(2, 0);
  }
}
