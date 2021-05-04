

// Digital ports
int dataPin = 2;


#define MCL_TIME_MS 3125
#define MCL_DATA_T1 1
#define MCL_DATA_T2 2
#define MCL_DATA_T3 3
#define MCL_ETX 4
#define MCL_STX 5


#define BUFFER_SIZE 1024
uint8_t str_buf[BUFFER_SIZE];
volatile int in_p = 0;
volatile int out_p = 0;

void setup() {

  // General
  // Serial port
  Serial.begin(230400);
  Serial.println("Data grabber ready");

  // B&O I/O's
  pinMode(dataPin, INPUT);      // Serial data

  // Setup timer and interrupt
  attachInterrupt(digitalPinToInterrupt(dataPin), pinChange, FALLING);


}
volatile int state = 0; uint8_t inbyte = 0x00;

int last_start = 0;

void loop(void) {
  while (1) {
    if (in_p < out_p) {
      if (!str_buf[in_p]) {
        dump_packet(last_start, in_p);
        last_start = in_p + 1;
      }
      in_p++;
    }
  }
}

void dump_packet(int start, int end) {
  auto i = start;
  while (i <= end) {
    uint8_t data = str_buf[i];
    Serial.write(data & 0xF0 ? " 0x" : " 0x0");
    Serial.print(data, HEX);
    i++;
  }
  Serial.write("\n");
  
  switch(str_buf[start]) {
    case(0xF0):
      Serial.write("[CD status message, track: ");
      Serial.print((str_buf[start+5] & 0x7F) >> 3);
      Serial.write("]\n");
  }
}

volatile int processing_message = 0;

#define WAITING_FOR_START_BIT 0
#define READING_BITS 1

volatile int current_bit = 0;

unsigned char reverse(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

void serialEvent() {
  Serial.print("X");
  Serial.flush();
}

void pinChange(void) {
  detachInterrupt(digitalPinToInterrupt(dataPin));
  //interrupts();

  for (int i = 0; i != 8; i++) {
    delayMicroseconds(3097); // Timed experimentally
    inbyte = (inbyte << 1) | !digitalRead(dataPin);
  }
  
  str_buf[out_p++] = inbyte;

  attachInterrupt(digitalPinToInterrupt(dataPin), pinChange, FALLING);
}
