#define DATA_PIN 2
#define MCL_TIME_MS 3125
#define MCL_PULSE_WIDTH 1562
#define MCL_DATA_T1 0
#define MCL_DATA_T2 1
#define MCL_DATA_T3 2
#define MCL_ETX 3
#define MCL_STX 4

volatile int last_bit = 0;
const char *up =   "0011101111000001010010001000110000000110"; // power on, to RADIO
const char *down = "0011101111000001010000000000001000000000"; // power off

enum Source {
  CD = 0x12,
  Phono = 0x13,
};

void setup() {
  Serial.begin(230400);
  pinMode(DATA_PIN, OUTPUT);      // Serial data
  
  send_preamble();
  send_preamble();
  send_stx();
  int i = 0;
  while (up[i]) {
    send_bit(up[i++] == '1' ? 1 : 0);
  };
  send_etx();
}

int encode_beolink_symbol(int this_bit, int previous_bit) {
  if (previous_bit) {
    return this_bit ? MCL_DATA_T2 : MCL_DATA_T1;
  } else {
    return this_bit ? MCL_DATA_T3 : MCL_DATA_T2;
  }
}

void send_pulse() {
  digitalWrite(DATA_PIN, LOW);
  delayMicroseconds(MCL_PULSE_WIDTH);
  digitalWrite(DATA_PIN, HIGH);
  delayMicroseconds(MCL_PULSE_WIDTH);
}

void send_stx() {
  delayMicroseconds(MCL_TIME_MS * MCL_STX);
  last_bit = 1;
  send_pulse();
}

void send_etx() {
  delayMicroseconds(MCL_TIME_MS * MCL_ETX);
  send_pulse();
}

void send_bit(int bit) {
  delayMicroseconds(MCL_TIME_MS * encode_beolink_symbol(bit, last_bit));
  last_bit = bit;
  send_pulse();
}

inline void send_preamble() {
  send_pulse();
}

uint8_t lol = 1;
void send_trk(uint8_t track) {
  send_bits(track, 8);

}

void send_bits(unsigned int data, int bits) {
  while (bits--) send_bit(data >> bits & 1);
}

void send_src(uint8_t src) {
  send_bits(src, 5);
}

void source_status(uint8_t src, uint8_t trk) {
  send_preamble();
  send_preamble();
  send_stx();
  send_bits(0b00111011110, 11);
  send_src(src);

  send_bit(0);
  send_bit(1);
  send_bit(0);
  send_bit(0);

  send_bit(0);
  send_bit(0);
  send_bit(0);
  send_bit(0);

  send_bit(0);
  send_bit(0);
  send_bit(0);
  send_bit(0);

  send_bit(0);
  send_bit(0);
  send_bit(0); // Power off display
  send_bit(0);

  send_trk(trk);

  send_etx();
}

void volume_status(uint8_t volume) {
  send_preamble();
  send_preamble();
  send_stx();
  send_bits(0b0011101111000001, 16);
  send_bits(0b0100100001000000, 16);
  send_bits(volume & 0x2F, 8);
  send_etx();
}

void loop() {
  int i = 0;

  source_status(lol++&0x1F, 13);
  volume_status(37);

  //volume_status(0);
  //source_status(Source::Phono, 0);

  // put your main code here, to run repeatedly:

}
