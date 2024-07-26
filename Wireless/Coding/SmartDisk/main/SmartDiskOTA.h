#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

//first access point
const char* ssid_from_AP_1 = "Wawako's Phone";
const char* your_password_for_AP_1 = "mimisijui";
//second access point
const char* ssid_from_AP_2 = "minor's major" ;
const char* your_password_for_AP_2 = "jinalakolautani";
//third access point 
const char* ssid_from_AP_3 = "milermole" ;
const char* your_password_for_AP_3 = "mimisijui1113";

void configOTA(){


// starting wifi station
    Serial.println("initializing Station service for OTA");
  // starting wifi station
    wifiMulti.addAP("ssid_from_AP_1", "your_password_for_AP_1");
    wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
    wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

    Serial.println("Connecting Wifi...");
    if(wifiMulti.run() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi connected");
        Serial.println("IP address: ");
        Serial.println(WiFi.localIP());
    }
//
  // seting up arduinoOTA
  // Port defaults to 3232
  //ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("smartDisk");

  // No authentication by default
  ArduinoOTA.setPassword("mimisijui");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else  // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
  ArduinoOTA.begin();

  Serial.println();
  Serial.print("OTA Ready on IP address: ");
  Serial.println(WiFi.localIP());

}



// void otaTask(){
//     configOTA();
//     for(;;){
//        ArduinoOTA.handle(); 
//     }
// }
// add this to the setup in the main file
// TaskHandle_t otaUpdateHandle = NULL;

// xTaskCreate(otaTask, "OTAupdate", 96000 , NULL, NULL, otaUpdateHandle );
