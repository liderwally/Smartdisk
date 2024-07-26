This is the folder that contains the last prototype that i, unluckily used as my final presentation prototype

The hardware component of the design includes:
	1. Esp32s3 running at 240MHz dual core RISCV SOC processor with Wi-Fi capabilities 
	2. The type A female usb connector 
	3. Power & charger controller using NE555 timer
	4. Programming ports
	5. LED diodes for status indication
	6. SPST push button for interaction with the system

The Software component of the design includes:
	1. WIFI Access Point for configuration activity and connectivity to user,
	2. Captive Portal,
	3. FreeRTOS for providing multitasking capability of servers,
	4. USB Mass Storage Server,
	5. Web file server.




failures encountered that limited the implementation of the prototype
	1. **Overall speed of SD Card is slow** which led to multisystem(webserver, Usb and ftp not to work),
	2. SMB server passes the limit for embedded as it requires more than 8MBs to operate,
	3. Ordering constrains the ordering process was delayed so i had to make one locally.


The working of the prototype can be seen in the demonstration folder




