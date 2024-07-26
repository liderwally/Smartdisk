#include "ff.h" /* Declarations of FatFs API */

FATFS FatFs; /* FatFs work area needed for each volume */
FIL Fil;     /* File object needed for each open file */

void setup() {
  UINT bw;
  FRESULT fr;


  f_mount(&FatFs, "", 0); /* Give a work area to the default drive */

  fr = f_open(&Fil, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS); /* Create a file */
  if (fr == FR_OK) {
    f_write(&Fil, "It works!\r\n", 11, &bw); /* Write data to the file */
    fr = f_close(&Fil);                      /* Close the file */
    if (fr == FR_OK && bw == 11) {           /* Lights green LED if data written well */
      DDRB |= 0x10;
      PORTB |= 0x10; /* Set PB4 high */
    }
  }
}
void loop() {}
