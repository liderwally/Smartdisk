#include <FS.h>
#include <FSImpl.h>
#include <vfs_api.h>
#include "sd_defines.h"
#include "sdmmc_cmd.h"

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


#include <USB.h>
#include "USBMSC.h"
#include "sdMMCHelper.h"
#include "sdHelper.h"

File file;
File sfile;
USBMSC MSC;




void setup() {
  // SERIAL INIT

  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.setDebugOutput(true);
  DBG_OUTPUT_PORT.print('\n');

  //////////////////////////////////
  // Print a welcome message to the Serial port.
  DBG_OUTPUT_PORT.println("SmartDisk 2.0");
  DBG_OUTPUT_PORT.println("\n\nAvailable features:  SD + USB Mass Storage Server + Indicator , V0.5.0 compiled " __DATE__ " " __TIME__ " by liderwally");  //__DATE__ is provided by the platformio ide
  DBG_OUTPUT_PORT.printf("%s-%d\n\r", ESP.getChipModel(), ESP.getChipRevision());
  SDInitiated = initSD();
  while (!SDInitiated)
    ;

  pinMode(9, INPUT);

  if (!digitalRead(9)) {

    MSC.vendorID("Programm");    //max 8 chars
    MSC.productID("USB_MSC");    //max 16 chars
    MSC.productRevision("1.0");  //max 4 chars
    MSC.onStartStop(onStartStop);
    //MSC.onEvent(usbEventCallback);
    MSC.onRead(&onRead);
    MSC.onWrite(&onWrite);
    MSC.begin((ESP.getFlashChipSize() / 4096), 4096);
    MSC.mediaPresent(true);
    USB.begin();
    USB.onEvent(usbEventCallback);
    DBG_OUTPUT_PORT.println("internal storage begin BEGIN!");

    // ESP.flashRead(uint32_t offset, uint32_t * data, size_t size);
    // ESP.flashRead(uint32_t offset, uint32_t * data, size_t size);
  } else {
    MSC.vendorID("uDISK");       //max 8 chars
    MSC.productID("USB_MSC");    //max 16 chars
    MSC.productRevision("1.0");  //max 4 chars
    MSC.onStartStop(onStartStop);
    //MSC.onEvent(usbEventCallback);
    MSC.onRead(&onRead);
    MSC.onWrite(&onWrite);
    MSC.begin((SD_MMC.totalBytes() / 512), 512);
    MSC.mediaPresent(true);
    USB.begin();
    USB.onEvent(usbEventCallback);
    DBG_OUTPUT_PORT.println("USB MMC BEGIN!");
  }
}

void loop() {
}



long onWrite(uint32_t lba, uint32_t offset, unsigned char *buffer, uint32_t bufsize) {
  bool err = true;
  uint8_t writeBuffer[4096];

if (digitalRead(9)) {
  int fullSectors = bufsize / 512;   // Number of full 512-byte sectors
  int remainingBytes = bufsize % 512; // Remaining bytes after full sectors

  // Process full 512-byte sectors
  for (int i = 0; i < fullSectors; i++) {
    memcpy(writeBuffer, (uint8_t *)buffer + 512 * i, 512);
    err &= SD_MMC.writeRAW((uint8_t *)writeBuffer, lba + i + offset);
  }

  // Process remaining bytes, if any
  if (remainingBytes > 0) {
    memset(writeBuffer, 0, 512); // Clear the write buffer
    memcpy(writeBuffer, (uint8_t *)buffer + 512 * fullSectors, remainingBytes);
    err &= SD_MMC.writeRAW((uint8_t *)writeBuffer, lba + fullSectors + offset);
  }
} else {
    err &= ESP.flashWrite(lba + offset, (uint32_t *)buffer, bufsize);
  }

  DBG_OUTPUT_PORT.printf("MSC WRITE ");
  return (err) ? (long int)bufsize : -1;

}

long onRead(uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize) {
  bool err = true;
  if (digitalRead(9)) {

    uint8_t readBuffer[4096];
    if (bufsize > 512) {
      for (int i = 0; i < bufsize / 512; i++) {
        err &= SD_MMC.readRAW((uint8_t *)readBuffer + (512 * i), lba + offset + i);
        memcpy(buffer, readBuffer, 512);
      }
    } else {
      err &= SD_MMC.readRAW((uint8_t *)readBuffer, lba + offset);
      memcpy(buffer, readBuffer, 512);
    }
  } else {
    err &= ESP.flashRead(lba, (uint32_t *)buffer, bufsize);
  }

  DBG_OUTPUT_PORT.printf("MSC READ");
  return (err) ? (long int)bufsize : -1;
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
        DBG_OUTPUT_PORT.println("USB PLUGGED");
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        DBG_OUTPUT_PORT.println("USB UNPLUGGED");
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        DBG_OUTPUT_PORT.printf("USB SUSPENDED: ");
        break;
      case ARDUINO_USB_RESUME_EVENT:
        DBG_OUTPUT_PORT.println("USB RESUMED");
        break;
      default:
        break;
    }
  }
}
