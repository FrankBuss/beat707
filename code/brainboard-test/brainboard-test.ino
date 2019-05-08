#include <SPI.h>

// buttons
#define SW1_PIN A4
#define SW2_PIN A3
#define SW3_PIN A2
#define SW4_PIN A1

// rotary encoder
#define ENCODER_A_PIN 3
#define ENCODER_B_PIN 2

// FRAM
#define CS_PIN 10

// TRS connector
#define CLICK_OUT_PIN 4
#define FOOTSWITCH_PIN A5

// TM1638 header
#define TM1638_STB_PIN 8
#define TM1638_CLK_PIN 9
#define TM1638_DIO_1_PIN 5
#define TM1638_DIO_2_PIN 6
#define TM1638_DIO_3_PIN 7

// test pin at J7, pin 23
#define TEST_PIN A0


// 7 segment numbers
const byte numbers[10] = { B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01100111};

// mapping of keyboard inputs
const uint8_t keyMap[32] = {
  0, 8, 16, 0, 4, 12, 20, 0,
  1, 9, 17, 0, 5, 13, 21, 0,
  2, 10, 18, 0, 6, 14, 22, 0,
  3, 11, 19, 0, 7, 15, 23, 0,
};

// encoder position
uint8_t g_position;

// table for all state transitions of the rotary encoder inputs,
// mapped to the counting direction
// A oldA B oldB counter
// 0 0    0 0    0
// 0 0    0 1    1
// 0 0    1 0    -1
// 0 0    1 1    0
// 0 1    0 0    -1
// 0 1    0 1    0
// 0 1    1 0    0
// 0 1    1 1    1
// 1 0    0 0    1
// 1 0    0 1    0
// 1 0    1 0    0
// 1 0    1 1    -1
// 1 1    0 0    0
// 1 1    0 1    -1
// 1 1    1 0    1
// 1 1    1 1    0
#if 0
// table of a perfect rotary encoder, without bouncing
static const int8_t g_delta2[16] = {
  0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0
};
#else
// for ALPS EC11 und EC12 Encoder:
// Ignore changes on B and use changes on A, only, if B=0
// this results in one step per raster and no bouncing between two values because of B
// see http://www.mikrocontroller.net/articles/Drehgeber for details
static const int8_t g_delta[16] = {
  0, 0, 0, 0, -1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
};
#endif

// returns the direction the rotary encoder was rotated: -1, 1 oder 0
static int8_t getDelta(uint8_t a, uint8_t oldA, uint8_t b, uint8_t oldB) {
  return g_delta[oldB | (b << 1) | (oldA << 2) | (a << 3)];
}

// should be called with 1 kHz or faster
void readEncoder() {
  // evaluate rotary encoder
  static uint8_t oldA = 0;
  static uint8_t oldB = 0;
  uint8_t a = digitalRead(ENCODER_A_PIN);
  uint8_t b = digitalRead(ENCODER_B_PIN);
  int8_t delta = getDelta(a, oldA, b, oldB);
  if (delta < 0) g_position--;
  if (delta > 0) g_position++;
  oldA = a;
  oldB = b;
}

void error(const char* text) {
  Serial.println(text);
  while (1) {}
}

void sendOneByteCommand(uint8_t command) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(command);
  digitalWrite(CS_PIN, HIGH);
}

void readId() {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x9f);
  uint16_t manufacturerId = SPI.transfer(0) << 8;
  manufacturerId |= SPI.transfer(0);
  uint16_t productId = SPI.transfer(0) << 8;
  productId |= SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);

  Serial.print("manufacturer ID: 0x");
  Serial.print(manufacturerId, HEX);
  Serial.print(", product ID: 0x");
  Serial.println(productId, HEX);
  if (manufacturerId == 0x047f && productId == 0x4803) {
    Serial.println("ID ok");
  } else {
    error("wrong ID");
  }
}

void writeEnable() {
  sendOneByteCommand(0x06);
}

void writeAddress(uint32_t address) {
  SPI.transfer((address >> 16) & 0xff);
  SPI.transfer((address >> 8) & 0xff);
  SPI.transfer(address & 0xff);
}

// write a byte
void writeByte(uint32_t address, uint8_t data) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x02);
  writeAddress(address);
  SPI.transfer(data);
  digitalWrite(CS_PIN, HIGH);
}

// read a byte
uint8_t readByte(uint32_t address) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(0x03);
  writeAddress(address);
  uint8_t data = SPI.transfer(0);
  digitalWrite(CS_PIN, HIGH);
  return data;
}

#define PAGE_SIZE 256

void framTest() {
  Serial.println("FRAM test...");
  SPI.begin();
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  readId();
  writeEnable();

  // program a page with test data
  for (int i = 0; i < PAGE_SIZE; i++) {
    writeByte(i, 0xff - i);
  }

  // verify programmed data
  for (int i = 0; i < PAGE_SIZE; i++) {
    if (readByte(i) != 0xff - i) {
      error("verify error");
    }
  }

  // program a page with 0
  for (int i = 0; i < PAGE_SIZE; i++) {
    writeByte(i, 0);
  }

  // verify programmed data
  for (int i = 0; i < PAGE_SIZE; i++) {
    if (readByte(i) != 0) {
      error("verify error");
    }
  }
  Serial.println("FRAM test ok");
}

void midiEchoTest() {
  static uint8_t fifo[256];
  uint8_t fifoRead = 0;
  uint8_t fifoWrite = 0;
  Serial.begin(31250);
  while (true) {
    if (Serial.available()) {
      fifo[fifoWrite++] = Serial.read();
    }
    if (fifoWrite != fifoRead) {
      Serial.write(fifo[fifoRead++]);
    }
  }
}

void setup() {
  // uncomment this for a MIDI test
  //midiEchoTest();
  
  // connect a 5 V serial port to J9 instead of using the jumper for MIDI output, for debug output
  Serial.begin(115200);
  Serial.println("brain board test");
  
  // switches
  pinMode(SW1_PIN, INPUT);
  pinMode(SW2_PIN, INPUT);
  pinMode(SW3_PIN, INPUT);
  pinMode(SW4_PIN, INPUT);

  // encoder
  pinMode(ENCODER_A_PIN, INPUT);
  pinMode(ENCODER_B_PIN, INPUT);

  // click out
  pinMode(CLICK_OUT_PIN, OUTPUT);

  // FRAM
  pinMode(CS_PIN, OUTPUT);
  framTest();

  // TM1638
  digitalWrite(TM1638_STB_PIN, HIGH);
  digitalWrite(TM1638_CLK_PIN, HIGH);
  pinMode(TM1638_STB_PIN, OUTPUT);
  pinMode(TM1638_CLK_PIN, OUTPUT);
  pinMode(TM1638_DIO_1_PIN, OUTPUT);
  pinMode(TM1638_DIO_2_PIN, OUTPUT);
  pinMode(TM1638_DIO_3_PIN, OUTPUT);

  // test pin
  pinMode(TEST_PIN, OUTPUT);
}

uint8_t tm1638Send(uint8_t b) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(TM1638_DIO_1_PIN, (b & 1) ? HIGH : LOW);
    digitalWrite(TM1638_CLK_PIN, LOW);
    digitalWrite(TM1638_CLK_PIN, HIGH);
    b >>= 1;
    if (digitalRead(TM1638_DIO_1_PIN) == HIGH) b |= 128;
  }
  return b;
}

uint8_t tm1638Read() {
  uint8_t temp = 0;

  for (int i = 0; i < 8; i++) {
    temp >>= 1;

    digitalWrite(TM1638_CLK_PIN, LOW);

    if (digitalRead(TM1638_DIO_1_PIN)) {
      temp |= 0x80;
    }

    digitalWrite(TM1638_CLK_PIN, HIGH);
  }

  return temp;
}

void loop() {
  // show switch changes on debug port
  static uint8_t oldSwitches = 0;
  uint8_t switches = 0;
  if (digitalRead(SW1_PIN) == LOW) switches |= 1;
  if (digitalRead(SW2_PIN) == LOW) switches |= 2;
  if (digitalRead(SW3_PIN) == LOW) switches |= 4;
  if (digitalRead(SW4_PIN) == LOW) switches |= 8;
  if (switches != oldSwitches) {
    oldSwitches = switches;
    Serial.print("switches: ");
    for (int i = 3; i >= 0; i--) {
      Serial.print(bitRead(switches, i));
    }
    Serial.println();
  }

  // show encoder changes on debug port
  readEncoder();
  static uint8_t oldPosition = 0;
  if (oldPosition != g_position) {
      Serial.print("encoder: ");
      Serial.println(g_position);
      oldPosition = g_position;
  }

  // click out test: 100 ms high each second
  digitalWrite(CLICK_OUT_PIN, (millis() % 1000) < 100);

  // footswitch test
  static uint8_t lastFootswitch = 0;
  uint8_t footswitch = digitalRead(FOOTSWITCH_PIN);
  if (footswitch != lastFootswitch) {
    Serial.print("footswitch: ");
    Serial.println(footswitch);
    lastFootswitch = footswitch;
  }


  // TM1638 test

  // max brightness
  digitalWrite(TM1638_STB_PIN, LOW);
  tm1638Send(0x88+7);
  digitalWrite(TM1638_STB_PIN, HIGH);

  // auto increment mode
  digitalWrite(TM1638_STB_PIN, LOW);
  tm1638Send(0x40);
  digitalWrite(TM1638_STB_PIN, HIGH);

  // start at address 0xc0
  digitalWrite(TM1638_STB_PIN, LOW);
  tm1638Send(0xc0);

  // send data
  static uint16_t led = 1;
  uint16_t l = led;
  for (int i = 0; i < 4; i++) {
    tm1638Send(numbers[i]);
    tm1638Send((l & 1) | ((l >> 7) & 2));
    l >>= 1;
  }
  for (int i = 0; i < 4; i++) {
    tm1638Send(numbers[i + 4]);
    tm1638Send((l & 1) | ((l >> 7) & 2));
    l >>= 1;
  }
  static uint8_t ledDelay = 0;
  if (ledDelay++ == 0) {
    led <<= 1;
  }
  if (led == 0) led = 1;
  digitalWrite(TM1638_STB_PIN, HIGH);

  // read keys
  digitalWrite(TM1638_STB_PIN, LOW);
  tm1638Send(0x42);
  pinMode(TM1638_DIO_1_PIN, INPUT);
  digitalWrite(TM1638_DIO_1_PIN, LOW);
  delay(1);
  digitalWrite(TEST_PIN, 1);
  uint32_t keys = tm1638Read();
  keys |= uint32_t(tm1638Read()) << 8;
  keys |= uint32_t(tm1638Read()) << 16;
  keys |= uint32_t(tm1638Read()) << 24;
  digitalWrite(TEST_PIN, 0);
  digitalWrite(TM1638_STB_PIN, HIGH);
  pinMode(TM1638_DIO_1_PIN, OUTPUT);

  // show first key
  int key = -1;
  for (int i = 0; i < 32; i++) {
    if (bitRead(keys, i)) {
      key = i;
      break;
    }
  }
  static int oldKey = -1;
  if (key != oldKey) {
    oldKey = key;
    if (key >= 0) {
      key = keyMap[key];
      Serial.println(key);
    }
  }
}
