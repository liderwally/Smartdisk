This is a four-bit wide data bus MMC memory test 

this example can be easily be implemented on locally available sd_card as they can be implemented using MMC protocol as well as SPI protocol 

To change to 8bit wide bus consider the following 
	1. Make sure the card can work on the lower bit bus first. i.e test the memory in 1bit then 4bit then 8bit for accuracy
	2. If you are working on SDCARD use 4bit but if its eMMC use 8Bit for efficiency 
	3. If you are using SDCARD use 40MHz max and if its eMMC use the specified speed on the datasheet
	4. If you're using the eMMC most of the pins are not define for any connection so use them to passthough your data bus or use the default component i designed for simplicity i included it in the PCB design folder

