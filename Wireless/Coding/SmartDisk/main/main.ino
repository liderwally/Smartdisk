#include "esp_system.h"
#include "mbedtls/md5.h"
#include <AsyncEventSource.h>
// #include <AsyncJson.h>
#include <AsyncWebSocket.h>
#include <AsyncWebSynchronization.h>
#include <ESPAsyncWebServer.h>
#include <StringArray.h>
// #include <WebAuthentication.h>
#include <WebHandlerImpl.h>
#include <WebResponseImpl.h>

#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>
#include "sd_defines.h"
#include "sdmmc_cmd.h"
#include <esp_task_wdt.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <DNSServer.h>  //for ip allocations
                        // #include <ESPmDNS.h>    //for host name
// #include "SmartDiskOTA.h"





String mainPath = "";
bool SDInitiated = false;
#if ARDUINO_USB_MODE
#include "USB.h"
#include "USBMSC.h"
#if ARDUINO_USB_CDC_ON_BOOT
#define HWSerial Serial0
#define USBSerial Serial
#else
#define HWSerial Serial
#endif
#define DBG_OUTPUT_PORT HWSerial
#else
#define DBG_OUTPUT_PORT Serial
#endif


//for usb msc

#include <USB.h>
#include "USBMSC.h"
#include "FirmwareMSC.h"



const char *webUser = "admin";
const char *webPass = "UDSM_SmartDisk";
// allows you to set the realm of authentication Default:"Login Required"
const char *www_realm = "Admin Realm";
// the Content of the HTML response in case of Unautherized Access Default:empty
String authFailResponse = "Authentication Failed";
String unsupportedFiles;

// defining machine states
#define IDLE 0x01
#define BUSSY 0x02
#define VERY_BUSSY 0x03
#define ERROR 0x04
#define UNSOLVED 0x05
#define SUCCESS 0x06

//defining usb states
#define USB_PLUGGED 0x01
#define USB_REMOVED 0x02
#define USB_STARTED 0x03
#define USB_SUSPENDED 0x04
#define USB_RESUMED 0x05


/* variables for storing machine states
we use memory to make sure the changes are made only 
if machine state is different. To reduce reccursive checking 
of machine state
*/
size_t machineState = IDLE;      // for storing current state
size_t lastMachineState = 0x00;  //memory for storing last state
float stateHue = 0;
size_t usbState = 

#ifdef ARDUINO_MAIN
#include <Arduino_FreeRTOS.h>
#include <freertos/task.h>
#else
#include <FreeRTOS.h>
#include <freertos/task.h>
#endif

uint64_t period = 1000;
float duty = 0.66;

void TaskServer(void *pvParameters);
void TaskRGB(void *pvParameters);
void otaTask(void *pvParameters);
#include "SPI.h"
// #include "SdFat.h"

// #include "USBMSC.h"

// extern "C" {
// #include <./smb2.h>
// #include "./libsmb2.h"
// }

// #include "html.h"
// #include "sdCard.h"

// WebServer server(80);

AsyncWebServer server(80);
DNSServer dnsServer;



#define STASSID "SmartDisk"
#define STAPSK NULL
#define MAX_CLIENTS 8   //
#define WIFI_CHANNEL 6  // 2.4ghz channel 6
#define ENABLE_USB_MODE true

const char *ssid = STASSID;
const char *password = STAPSK;
const char *host = "SmartDisk";
const IPAddress localIP(8, 8, 4, 4);           // the IP address the web server, Samsung requires the IP to be in public space
const IPAddress gatewayIP(8, 8, 4, 4);         // IP address of the network should be the same as the local IP for captive portals
const IPAddress subnetMask(255, 255, 255, 0);  // no need to change: https://avinetworks.com/glossary/subnet-mask/
const String localIPURL = "http://8.8.4.4";    // a string version of the local IP with http, used for redirecting clients to your webpage
int sdCardSize, sdSectorSize, sdNumSectors;



#include "webComponents.h"
// #include "icons.h"

#include "sdMMCHelper.h"
#include "sdHelper.h"

File file;
File sfile;

#include "severHelper.h"
#include "indicator.h"


void startUsb();
//*******************************************************
//************      setup       **************************
//**********************************************************
void setup() {

  ////////////////////////////////
  // SERIAL INIT

  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print('\n');

  //////////////////////////////////
  // Print a welcome message to the Serial port.
  DBG_OUTPUT_PORT.println("SmartDisk 1.0");
  DBG_OUTPUT_PORT.println("\n\nAvailable features: WiFi + DNS Server + Captive portal + SD + USB Mass Storage Server + Indicator , V0.5.0 compiled " __DATE__ " " __TIME__ " by liderwally");  //__DATE__ is provided by the platformio ide
  DBG_OUTPUT_PORT.printf("%s-%d\n\r", ESP.getChipModel(), ESP.getChipRevision());
  DBG_OUTPUT_PORT.printf("Connect sd card to the following pins >> MISO: %d , MOSI: %d, SS: %d , SCK: %d", MISO, MOSI, SS, SCK);
  ////////////////////////////////
  ///// FILESYSTEM INIT

  //SD Card initiate
  SDInitiated = initSD();
  if (!SDInitiated) {
    DBG_OUTPUT_PORT.println("Failed to initiate the SD card file system");
  } else {
    DBG_OUTPUT_PORT.printf("Number of sectors for SD : %d each contains %d ", sdCardSize, sdSectorSize);
  }
  if (!SD_MMC.exists("/Shared")) {
    if (createDir(SD_MMC, "/Shared")) {
      DBG_OUTPUT_PORT.printf("The shared folder created successfully");
    } else {
      DBG_OUTPUT_PORT.printf("Failed to create a shared folder");
    };
  } else {
    DBG_OUTPUT_PORT.printf("The shared folder created successfully");
  };


  sdCardSize = SD_MMC.totalBytes();
  sdSectorSize = SD_MMC.sectorSize();
  sdNumSectors = SD_MMC.numSectors();
  sfile = SD_MMC.open("/Shared");

  //SPIFF initiate
  Serial.println("Mounting SPIFFS ...");



  if (!SPIFFS.begin(true)) {
    // if you have not used SPIFFS before on a ESP32, it will show this error.
    // after a reboot SPIFFS will be configured and will happily work.
    Serial.println("ERROR: Cannot mount SPIFFS, ");
  } else {
    DBG_OUTPUT_PORT.printf("Number of sectors for SPIFFS: %d ", (int)SPIFFS.totalBytes());
  }

  //enable mass storage server if usb mode is enabled
  if (ENABLE_USB_MODE) {

    startUsb();
  }



  ////////////////////////////////
  // WI-FI INIT


  DBG_OUTPUT_PORT.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password, WIFI_CHANNEL, 0, MAX_CLIENTS);
  WiFi.softAPConfig(localIP, gatewayIP, IPAddress(255, 255, 255, 0));

  DBG_OUTPUT_PORT.print("AP IP address: ");
  DBG_OUTPUT_PORT.println(localIP);

  ////////////////////////////////
  // DNS Server INIT


  dnsServer.setTTL(3600);
  dnsServer.start(53, "*", localIP);  //DNSport = 53, "*"- server will serve to all ips
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp32.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network

  // if ( !MDNS.begin("smartdisk") ) {
  //   DBG_OUTPUT_PORT.println("Error setting up MDNS responder!");
  //   while (1) {
  //     delay(1000);
  //   }
  // }
   DBG_OUTPUT_PORT.println("mDNS responder started");

  ////////////////////////////////
  // WEB SERVER INIT
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);  //only when requested from AP
  // Required
  server.on("/connecttest.txt", [](AsyncWebServerRequest *request) {
    request->redirect("http://logout.net");
  });  // windows 11 captive portal workaround
  server.on("/wpad.dat", [](AsyncWebServerRequest *request) {
    request->send(404);
  });  // Honestly don't understand what this is but a 404 stops win 10 keep calling this repeatedly and panicking the esp32 :)

  // Background responses: Probably not all are Required, but some are. Others might speed things up?
  // A Tier (commonly used by modern systems)
  server.on("/generate_204", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });  // android captive portal redirect
  server.on("/redirect", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });  // microsoft redirect
  server.on("/hotspot-detect.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });  // apple call home
  server.on("/canonical.html", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });  // firefox captive portal call home
  server.on("/success.txt", [](AsyncWebServerRequest *request) {
    request->send(200);
  });  // firefox captive portal call home

  server.on("/ncsi.txt", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });  // windows call home

  // B Tier (uncommon)
  server.on("/chrome-variations/seed", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });  //chrome captive portal call home
  server.on("/service/update2/json", [](AsyncWebServerRequest *request) {
    request->send(200);
  });  //firefox?
  server.on("/chat", [](AsyncWebServerRequest *request) {
    request->send(404);
  });  //No stop asking Whatsapp, there is no internet connection
  server.on("/startpage", [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL);
  });

  // return 404 to webpage icon
  server.on("/favicon.ico", [](AsyncWebServerRequest *request) {
    request->send(404);
  });  // webpage icon

  server.on("/api", [](AsyncWebServerRequest *request) {
    request->send(404);
  });  // webpage icon


  // // Serve on root
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->redirect(localIPURL + "/index");
  });

  // //server index
  server.on("/index", HTTP_GET, [](AsyncWebServerRequest *request) {
    handleIndex(request);
  });
  ////server file upload
  server.onFileUpload(handleFileUpload);

  // // // // server icon request
  // // // server.on("/icons", HTTP_GET, handleIcons

  // // server initial request
  server.on("/admin", HTTP_ANY, handleIndexAdmin);

  // // // Filesystem status
  server.on("/status", HTTP_GET, handleStatus);

  // // // List directory
  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    machineState = BUSSY;
    handleFileList(request);
  });

  // get file
  server.on("/server", HTTP_GET, [](AsyncWebServerRequest *request) {
    DBG_OUTPUT_PORT.println(request->url());
    handleFileRead(request, &SD_MMC, String(request->url()), true);
  });




  DBG_OUTPUT_PORT.print("The content type of json file is : ");
  DBG_OUTPUT_PORT.println(getContentType("index.json"));
  // // Create file
  server.on("/add", HTTP_GET, handleFileCreate);
  server.on("/rename", HTTP_GET, [](AsyncWebServerRequest *request) {
    String secPath = request->arg("comm2");
    // secPath = secPath.strip();

    if (secPath.indexOf('/') < 0 || secPath.indexOf('.') < 0) {
      replyBadRequest(request, "Destination is not well defined!");
    } else {
      handleFileCreate(request);
    }
  });

  // // // // Delete file
  server.on("/delete", HTTP_DELETE, handleFileDelete);

  // // // // Upload file
  // // // // - first callback is called after the request has ended with all parsed arguments
  // // // // - second callback handles file upload at that location
  // server.on("/upload", HTTP_POST, replyOK, []() {
  //   handleFileUpload();
  // });

  // Default handler for all URIs not defined above
  // Use it to read files from filesystem
  server.onNotFound([](AsyncWebServerRequest *request) {
    machineState = ERROR;
    handleNotFound(request);
  });

  //serve static on specific folder
  // server.serveStatic("/index", SD, "/server");
  // Start server
  server.begin();
  // MDNS.addService("http", "tcp", 80);

  DBG_OUTPUT_PORT.println("HTTP server started");

  // Open root directory
  if (SD_MMC.open("/Shared")) {
    DBG_OUTPUT_PORT.println(" fileNotFound file is present");
    file = SD_MMC.open("/Shared", "w");
    file.println("The folder youre trying to access is not present");

    // Serial.println(file.readString());
    size_t filesize = file.size();
    Serial.println(filesize);
    file.close();
  }

  digitalWrite(RGB_BUILTIN, HIGH);

  // #include "mscdisk.h"


  //test the rgb light
  testRGB();

  // xTaskCreatePinnedToCore(
  //   TaskServer,
  //   "Blink",
  //   128 * 2,
  //   NULL,
  //   tskIDLE_PRIORITY,
  //   NULL, 1);

  xTaskCreate(
    TaskRGB,
    "neopixel LED",
    32000,
    NULL,
    1,
    NULL);

//   xTaskCreate(
//     otaTask,
//     "OTAupdate",
//     25600,
//     NULL,
//     1,
//     NULL);
}

void loop() {
  DBG_OUTPUT_PORT.println(".");
  delay(300);
}



///////////////////////////////////////////////////////////////////
/////////// OTHER FUNCTIONS //////////////////////////////////////
/////////////////////////////////////////////////////////////////


void TaskRGB(void *pvParameters) {
  while (true) {
    // defining machine state
    if (machineState != lastMachineState) {
      switch (machineState) {
        case BUSSY:
          stateHue = 0.6;
          period = 2000;
          duty = 0.2;
          break;
        case VERY_BUSSY:
          stateHue = 0.4;
          period = 1000;
          duty = 0.6;
          break;
        case ERROR:
          stateHue = 0.2;
          period = 500;
          duty = 0.5;
          break;
        case SUCCESS:
          stateHue = 0.49;
          period = 4000;
          duty = 0.8;
          break;
        default:
          stateHue = 6;
          period = 4000;
          duty = 0.8;
          break;
      }
      lastMachineState = machineState;
    }
    // indicating machine state
    float deRem = millis() % period;
    if (deRem > 100 && deRem < (duty * period + 100)) {
      neopixelSetRGBHue(stateHue);
    } else {
      digitalWrite(RGB_BUILTIN, LOW);
    }
  }
}



static int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize) {
  uint32_t secSize = SD_MMC.sectorSize();
  if (!secSize) {
    return false;  // disk error
  }
  log_v("Write lba: %ld\toffset: %ld\tbufsize: %ld", lba, offset, bufsize);
  for (int x = 0; x < bufsize / secSize; x++) {
    uint8_t blkbuffer[secSize];
    memcpy(blkbuffer, (uint8_t *)buffer + secSize * x, secSize);
    if (!SD_MMC.writeRAW(blkbuffer, lba + x)) {
      return false;
    }
  }
  return bufsize;
}

static int32_t onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  uint32_t secSize = SD_MMC.sectorSize();
  if (!secSize) {
    return false;  // disk error
  }
  log_v("Read lba: %ld\toffset: %ld\tbufsize: %ld\tsector: %lu", lba, offset, bufsize, secSize);
  for (int x = 0; x < bufsize / secSize; x++) {
    if (!SD_MMC.readRAW((uint8_t *)buffer + (x * secSize), lba + x)) {
      return false;  // outside of volume boundary
    }
  }
  return bufsize;
}

bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  DBG_OUTPUT_PORT.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    // arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT:
        // DBG_OUTPUT_PORT.println("USB PLUGGED");
        usbState = USB_PLUGGED;
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        // DBG_OUTPUT_PORT.println("USB UNPLUGGED");
        usbState = USB_STOPPED;
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        // DBG_OUTPUT_PORT.printf("USB SUSPENDED: remote_wakeup_en: %u\n", data->suspend.remote_wakeup_en);
        usbState = USB_SUSPENDED;
        break;
      case ARDUINO_USB_RESUME_EVENT:
      usbState = USB_RESUMED;
        // DBG_OUTPUT_PORT.println("USB RESUMED");
        break;
      default:
        break;
    }
  }
}


void startUsb() {
  bool mscstate = digitalRead(9);
  if (mscstate || SD_MMC.exists("/")) {
    USBMSC MSC;
    // size_t totalBytes = SD_MMC.totalBytes();
    // size_t totalUsedBytes = SD_MMC.usedBytes();

    MSC.vendorID("uDISK");       //max 8 chars
    MSC.productID("USB_MSC");    //max 16 chars
    MSC.productRevision("1.0");  //max 4 chars
    MSC.onStartStop(onStartStop);
    // MSC.onEvent(usbEventCallback);
    MSC.onRead(&onRead);
    MSC.onWrite(&onWrite);
    MSC.mediaPresent(true);
    MSC.begin(SD_MMC.numSectors(), SD_MMC.sectorSize());
    MSC.mediaPresent(true);
  } else {

    FirmwareMSC MSC;
    MSC.onEvent(usbEventCallback);
    MSC.begin();
  }
  USB.begin();
}


// void otaTask(void *pvParameters) {
//   (void)pvParameters;
//   configOTA();
//   for (;;) {
//     ArduinoOTA.handle();
//   }
// }