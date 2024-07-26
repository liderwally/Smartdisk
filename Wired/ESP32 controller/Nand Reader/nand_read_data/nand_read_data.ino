#include <List.hpp>
uint8_t readBuff[4096];
uint8_t writeBuff[4096];
#include "nandflash.h"
#include "esp_task_wdt.h"

// #define INCLUDE_OTA


uint8_t nandWComStat;  // 0 = not done , 1 = working , 2 = done
uint8_t* destWBuffer;
uint8_t* sourceWBuffer;
List<uint32_t> operationWSize;
List<uint32_t> nandWLba;


uint8_t nandRComStat;  // 0 = not done/just assigned , 1 = working , 2 = done
uint8_t* destRBuffer;
uint8_t* sourceRBuffer;
List<uint32_t> operationRSize;
List<uint32_t> nandRLba;

void nandWriteServer(void* pvParameters) {
  (void)pvParameters;

  for (;;) {
    do{  //
      Serial.printf("nandWriteServer \n");
      nandWComStat = 1;
      uint32_t buffsize = operationWSize.get(0);
      uint32_t lba = nandWLba.get(0);
      //writing

      uint8_t blocks = lba / BLOCK_SIZE;
      uint8_t pages = lba % BLOCK_SIZE;
      nandWrite(0x00, pages, blocks, sourceWBuffer, buffsize);


      nandWComStat = 2;
      // destWBuffer = writeBuff;
      sourceWBuffer = writeBuff;
      operationWSize.removeFirst();
      nandWLba.removeFirst();
    }while ((!nandWLba.isEmpty()) && (sourceWBuffer < (writeBuff + 4096)));
  }
}

void nandReadServer(void* pvParameters) {
  (void)pvParameters;

  for (;;) {
    while ((!nandRLba.isEmpty()) && (destRBuffer < (readBuff + 4096))) {  //
      Serial.printf("nandReadServer \n");
      nandRComStat = 1;
      uint32_t buffsize = operationRSize.get(0);
      uint32_t lba = nandRLba.get(0);

      uint8_t blocks = lba / BLOCK_SIZE;
      uint8_t pages = lba % BLOCK_SIZE;
      // esp_task_wdt_init(300, false);
      nandReadPage(pages, blocks, destRBuffer, buffsize);

      nandRComStat = 2;
      destRBuffer = readBuff;
      // sourceRBuffer = readBuff;
      operationRSize.removeFirst();
      nandRLba.removeFirst();
    }
  }
}




void nandloop(void* pvParameters) {
  (void)pvParameters;
  if (Serial.available() > 0) {
    if (Serial.peek() == 'm') {
      ss = false;
      while (Serial.read() != -1)
        ;  //- to clear buffer
      Serial.println("Set read mode: main area.");
    } else if (Serial.peek() == 's') {
      ss = true;
      while (Serial.read() != -1)
        ;  // to clear buffer
      Serial.println("Set read mode: spare area.");
    } else {
      bs = Serial.parseInt();
      if ((bs < 0) || (bs >= 4095)) {
        Serial.println("Valid blocks are: 0..4095.");
      } else {
        uint8_t buffer[4096];
        uint8_t* ptBuffr = buffer;
        nandReadBlock(bs, buffer, 4096);
        for (int i = 0; i < 4096; i++) {
          Serial.printf("index:%d , value: %p ", i, ptBuffr);
        }
        printBlock(bs, ss);
      }
    }
  }
}

#define CFG_TUD_MSC_EP_BUFSIZE 4096
// uint8_t msc_disk[NUM_OF_SECTORS][SECTOR_SIZE];

#include "USB.h"
#include "USBMSC.h"


USBMSC MSC;


#define USB_PLUGGED 0x00
#define USB_UNPLUGGED 0x01
#define USB_SUSPENDED 0x02
#define USB_RESUMED 0x03
#define USB_ERROR 0x04

int usbState = USB_UNPLUGGED;

void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
bool onStartStop(uint8_t power_condition, bool start, bool load_eject);
int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);



void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  pinMode(NAND_RB, INPUT_PULLUP);
  pinMode(NAND_R, OUTPUT);
  pinMode(NAND_E, OUTPUT);
  pinMode(NAND_CL, OUTPUT);
  pinMode(NAND_AL, OUTPUT);
  pinMode(NAND_W, OUTPUT);




  nandIdleBus();

  Serial.println("NAND Test");
  nandEnable(true);
  nandReadSignature();
  nandReadStatus();

  Serial.println("NAND Test ");
  nandEnable(true);
  nandReadSignature();
  nandReadStatus();

  Serial.println("Please set \"No line ending\" in Serial Monitor");
  Serial.println("Send m/s for main/square memory area.");
  Serial.println("Send block address to read [0..4095].");

  USB.onEvent(usbEventCallback);
  MSC.vendorID("Smart");       //max 8 chars
  MSC.productID("udisk");      //max 16 chars
  MSC.productRevision("1.0");  //max 4 chars
  MSC.onStartStop(onStartStop);
  MSC.onRead(onRead);
  MSC.onWrite(onWrite);
  MSC.mediaPresent(true);
  MSC.begin(USABLE_SECTORS, SECTOR_SIZE);
  // MSC.begin(256, 4096);
  USB.begin();

#ifdef INCLUDE_OTA
#include "SmartDiskOTA.h"
  TaskHandle_t otaUpdateHandle;
  xTaskCreate(otaTask, "OTAupdate", 8192, &ArduinoOTA, 2, &otaUpdateHandle);
#endif
  TaskHandle_t nandReadServerHandle;
  TaskHandle_t nandWriteServerHandle;

  xTaskCreatePinnedToCore(nandReadServer, "NandWriteServer", 20000, NULL, 3, &nandReadServerHandle, 1);
  xTaskCreatePinnedToCore(nandWriteServer, "NandReadServer", 20000, NULL, 3, &nandWriteServerHandle, 1);


  TaskHandle_t dumpHandle;

  xTaskCreate(nandloop, "NandDump", 20000, NULL, 3, &dumpHandle);
}


int lastUsbState = 0x00;
void loop() {
  // nandloop(NULL);
  if (usbState != lastUsbState) {
    switch (usbState) {
      case USB_UNPLUGGED: Serial.printf("Usb -> UNPLUGGED"); break;
      case USB_PLUGGED: Serial.printf("Usb  -> PLUGGED"); break;
      case USB_SUSPENDED: Serial.printf("Usb  -> SUSPENDED"); break;
      case USB_RESUMED: Serial.printf("Usb  -> RESUMED"); break;
      case USB_ERROR: Serial.printf("Usb  -> ERROR"); break;
    }
    Serial.println();
    lastUsbState = usbState;
  }
}
// #endif /* ARDUINO_USB_MODE */





int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize) {

  // Serial.printf("MSC WRITE: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);
  memcpy(writeBuff, buffer, bufsize);

  // Serial.printf("nand Write : page %d , block , %d, buffersize : %d", pages, blocks, bufsize);
  esp_task_wdt_init(300, false);
  nandWComStat = 0;
  destWBuffer = NULL;
  sourceWBuffer = writeBuff;
  operationWSize.addLast(bufsize);
  nandWLba.addLast(lba + OFFSECTORS);
  // vTaskDelay(50 / portTICK_PERIOD_MS);
  // while (!nandWLba.isEmpty());
  return bufsize;
}

int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize) {
  size_t time = millis();
  Serial.printf("MSC READ: lba: %u, offset: %u, bufsize: %u\n", lba, offset, bufsize);



  // memcpy(buffer, msc_disk[lba] + offset, bufsize);
  // nandReadBlock(lba, (uint8_t*)buffer, bufsize);
  nandRComStat = 0;
  destRBuffer = readBuff;
  operationRSize.addLast(bufsize);
  nandRLba.addLast(lba + OFFSECTORS);
  // vTaskDelay(50 / portTICK_PERIOD_MS);
  // while (!nandRLba.isEmpty())
  //   ;
  memcpy(buffer, readBuff, bufsize);
  Serial.printf(" Done read : %d microseconds \n ", (int)(millis() - time));
  return bufsize;
}

bool onStartStop(uint8_t power_condition, bool start, bool load_eject) {
  Serial.printf("MSC START/STOP: power: %u, start: %u, eject: %u\n", power_condition, start, load_eject);
  return true;
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
  (void) lun;

  return ( (nandRComStat == 2) && (nandWComStat == 2) )? true: false; // RAM disk is always ready
}


void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
  if (event_base == ARDUINO_USB_EVENTS) {
    // arduino_usb_event_data_t* data = (arduino_usb_event_data_t*)event_data;
    switch (event_id) {
      case ARDUINO_USB_STARTED_EVENT:
        usbState = USB_PLUGGED;
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        usbState = USB_UNPLUGGED;
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        usbState = USB_SUSPENDED;
        break;
      case ARDUINO_USB_RESUME_EVENT:
        usbState = USB_RESUMED;
        break;

      default:
        break;
    }
  }
}
