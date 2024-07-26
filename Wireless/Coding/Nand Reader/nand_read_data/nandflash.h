


#define NAND_RB 41  // Read busy
#define NAND_R 40   // Read Enable
#define NAND_E 39   // Chip Enable
#define NAND_CL 35  // Command latch
#define NAND_AL 36  // Address latch
#define NAND_W 37   // Write
#define NAND_WP 38
#define NAND_IO0 8
#define NAND_IO1 18
#define NAND_IO2 17
#define NAND_IO3 16
#define NAND_IO4 15
#define NAND_IO5 7
#define NAND_IO6 6
#define NAND_IO7 5

#define BUS_WIDTH 8          // 1 byte
#define PAGE_SIZE 4096       // 4k bytes
#define PAGE_SPARE_SIZE 224  //bytes
#define BLOCK_SIZE 128       // pages => (128 * 4096 = 524288 bytes)
#define PLANE_SIZE 2048      //blocks  => (2048 * 128* 4096 = 8GB )
#define PLANE_NUMBER 2       // number of blocks present
#define NUM_OF_SECTORS (2048 * 128 * 2)
#define OFFSECTORS 4288
#define USABLE_SECTORS (NUM_OF_SECTORS - OFFSECTORS)
#define SECTOR_SIZE PAGE_SIZE

int ioPort[8] = { NAND_IO0, NAND_IO1, NAND_IO2, NAND_IO3, NAND_IO4, NAND_IO5, NAND_IO6, NAND_IO7 };

bool ss;     // spare select
int16_t bs;  // block index
uint32_t* nextPointer = NULL;
uint32_t* thisPointer = NULL;
uint32_t* destPointer = NULL;
uint32_t* sourcePointer = NULL;
uint32_t pointerCounter = 0;



void portModeOutput() {
  for (int i = 0; i < BUS_WIDTH; i++) {
    pinMode(ioPort[i], OUTPUT);
  }
  // Serial.println("Data out");
}

void portModeInput() {
  for (int i = 0; i < BUS_WIDTH; i++) {
    pinMode(ioPort[i], INPUT);
  }
  // Serial.println("Data in");
}

void portWrite(uint8_t by) {
  portModeOutput();
  for (int i = 0; i < BUS_WIDTH; i++) {
    digitalWrite(ioPort[i], (by >> i) & 0x01);
  }
  // Serial.printf("Data write: %d \n\r", by);
}

uint8_t portRead() {
  portModeInput();
  uint8_t by = 0x00;
  for (int i = 0; i < BUS_WIDTH; i++) {
    bool thisBit = digitalRead(ioPort[i]);
    by |= thisBit << (BUS_WIDTH - 2 - i);
  }
  // Serial.printf("Data read : %d \n\r", by);
  return by;
}

void nandAwait() {
  while (digitalRead(NAND_RB) == LOW)
    ;
}

void nandIdleBus() {
  portModeOutput();
  digitalWrite(NAND_R, HIGH);
  digitalWrite(NAND_W, HIGH);
  digitalWrite(NAND_CL, LOW);
  digitalWrite(NAND_AL, LOW);
  digitalWrite(NAND_E, HIGH);
}

void nandEnable(bool e) {
  portModeOutput();
  digitalWrite(NAND_E, !e);
}

void nandSendCommand(uint8_t cmd, bool last = true) {
  portModeOutput();
  digitalWrite(NAND_CL, HIGH);
  digitalWrite(NAND_W, LOW);
  portWrite(cmd);
  digitalWrite(NAND_W, HIGH);  // this triggers data capture
  if (last) digitalWrite(NAND_CL, LOW);
}

void nandSendAddress(uint8_t column, uint8_t page, uint16_t block) {
  int pageSize = PAGE_SIZE;
  int blockSize = BLOCK_SIZE;
  int planeSize = PLANE_SIZE;
  if (column >= pageSize || page >= blockSize || block >= planeSize) {
    Serial.println("invalid address");
    return;
  }
  digitalWrite(NAND_AL, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite(column);
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite((page & 0x1F) | ((block & 0x07) << 5));
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite((block >> 3) & 0xFF);
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite((block >> 11) & 0x03);
  digitalWrite(NAND_W, HIGH);

  digitalWrite(NAND_AL, LOW);
}

void nandReadSignature() {
  uint8_t sm = 0x00, sd = 0x00;

  portModeOutput();
  nandSendCommand(0x90);
  digitalWrite(NAND_AL, HIGH);
  digitalWrite(NAND_W, LOW);
  portWrite(0x00);             // address 0x00
  digitalWrite(NAND_W, HIGH);  // this triggers data capture
  digitalWrite(NAND_AL, LOW);

  portModeInput();
  digitalWrite(NAND_R, LOW);
  sm = portRead();
  digitalWrite(NAND_R, HIGH);
  digitalWrite(NAND_R, LOW);
  sd = portRead();
  digitalWrite(NAND_R, HIGH);

  Serial.print("NAND manufacturer: ");
  Serial.println(sm, HEX);
  Serial.print("NAND device ID:    ");
  Serial.println(sd, HEX);
}

void nandPrintStatus() {
  uint8_t st = 0x00;

  portModeOutput();
  nandSendCommand(0x70);
  portModeInput();

  // nandAwait();
  digitalWrite(NAND_R, LOW);
  st = portRead();
  digitalWrite(NAND_R, HIGH);

  Serial.print("NAND status:       ");
  Serial.println(st, DEC);
}

uint8_t nandReadStatus() {
  uint8_t st = 0x00;

  portModeOutput();
  nandSendCommand(0x70);
  portModeInput();
  // nandAwait();
  digitalWrite(NAND_R, LOW);
  st = portRead();
  digitalWrite(NAND_R, HIGH);

  return st;
}


void nandReadPageArea(uint16_t areaSize = PAGE_SIZE) {
  uint8_t d[areaSize];
  // uint8_t cd = 0;

  for (uint16_t i = 0; i < areaSize; i++) {
    digitalWrite(NAND_R, LOW);
    d[i] = portRead();
    digitalWrite(NAND_R, HIGH);
  }

  for (uint16_t i = 0; i < areaSize; i++) {
    if (i % 16 == 0) Serial.println();

    if (d[i] < 0x10) Serial.print('0');
    Serial.print(d[i], HEX);
    Serial.print(' ');
  }
}

void nandReadPage(uint16_t page, uint16_t block, uint8_t* buffer, uint32_t buffSize) {

  uint8_t* buffref = buffer;
  uint8_t* endbuff = buffer + buffSize;
  portModeOutput();
  nandSendCommand(0x00);
  nandSendAddress(0x00, page, block);
  nandSendCommand(0x30);
  portModeInput();
  nandAwait();
  //read for how many points

  do {

    //assign a value to the incremented point
    digitalWrite(NAND_R, LOW);
    *buffref = portRead();
    digitalWrite(NAND_R, HIGH);
    buffref++;
    // Serial.printf("PortRead: %d , index: %d , pointer : %p \n", *buffref, i, buffref);
  } while (buffref < endbuff);

  nandIdleBus();
}


void nandReadBlock(uint16_t block) {
  for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
    portModeOutput();
    nandSendCommand(0x00);
    nandSendAddress(0x00, i, block);
    portModeInput();

    while (digitalRead(NAND_RB) == LOW)
      ;
    nandReadPageArea();

    portModeOutput();
    nandSendCommand(0x01);
    nandSendAddress(0x00, i, block);
    portModeInput();
    nandAwait();
    nandReadPageArea();
  }
}

void nandReadBlock(uint16_t block, uint8_t* buffer, uint32_t buffSize) {
  uint8_t* buffref = buffer;
  Serial.printf("Now reading block : %d , starting address : %p", block, buffer);
  portModeOutput();
  nandSendCommand(0x00);
  nandSendAddress(0x00, 0x00, block);
  nandSendCommand(0x30);
  portModeInput();
  // nandAwait();
  //read for how many points
  for (uint8_t i = 0; i < buffSize; i++) {
    buffref = buffer;
    //increment points
    for (int j = 0; j < i; j++) {
      buffref++;
    }
    //assign a value to the incremented point

    *buffref = portRead();
    Serial.print(" address : ");
    Serial.printf("%p", buffref);
    Serial.print(" ,content : ");
    Serial.print(*buffref);
    Serial.println();
  }
  nandIdleBus();
}

void nandReadBlockSpare(uint16_t block) {
  for (uint8_t i = 0; i < BLOCK_SIZE; i++) {
    portModeOutput();
    nandSendCommand(0x50);
    nandSendAddress(0x00, i, block);
    portModeInput();
    nandAwait();
    nandReadPageArea(16);
  }
}

void startTiming(size_t* time) {
  *time = millis() - *time;
}

void endTiming(uint32_t* time, uint32_t byteSize) {
  *time = millis() - *time;

  Serial.println();
  Serial.println();
  Serial.print("Read time: ");
  Serial.print(*time);
  Serial.println(" milliseconds");

  Serial.print("Speed: ");
  Serial.print(byteSize * 32000.0 / *time / 1024, 2);
  Serial.println(" kilobytes/second");
  Serial.println();
  Serial.println();
}

void printBlock(uint16_t block, bool spare = false) {
  Serial.print("NAND Block ");
  Serial.print(block, DEC);
  Serial.println(" Dump:");

  unsigned long total_t = 0;
  unsigned long start_t = millis();

  // first block = 0x0000 (0)
  // last block  = 0x0FFF (4095)
  if (spare) nandReadBlockSpare(block);
  else nandReadBlock(block);

  total_t = millis() - start_t;

  Serial.println();
  Serial.println();
  Serial.print("Read time: ");
  Serial.print(total_t);
  Serial.println(" milliseconds");

  Serial.print("Speed: ");
  Serial.print(32 * PAGE_SIZE * 1000.0 / total_t / 1024, 2);
  Serial.println(" kilobytes/second");
}

void nandWrite(uint column, uint page, uint block, uint8_t* buffer, uint blocksize) {
  portModeOutput();
  nandSendCommand(0x80);
  // nandAwait();
  nandSendAddress(column, page, block);
  for (int i = 0; i < blocksize; i++) {
    digitalWrite(NAND_W, LOW);
    portWrite((int)*buffer + i);
    digitalWrite(NAND_W, HIGH);
  }
  nandSendCommand(0x10);
  // nandAwait();
  nandPrintStatus();
}

void nandReset() {
  portModeOutput();
  nandSendCommand(0xff);
  portModeInput();
}

bool nandChangeWriteColumn(uint column) {
  portModeOutput();
  nandSendCommand(0x85);
  if (column >= PAGE_SIZE) {
    Serial.println("invalid column");
    return false;
  }
  digitalWrite(NAND_AL, HIGH);

  digitalWrite(NAND_W, LOW);
  portWrite(column);
  digitalWrite(NAND_W, HIGH);

  return (nandReadStatus() == 0x00) ? false : true;
}


//0x3fc965e8