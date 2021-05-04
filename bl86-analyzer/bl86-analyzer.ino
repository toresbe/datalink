
#include <TimerOne.h>

// Digital ports
int dataPin = 2;


#define MCL_TIME_MS 3125
#define MCL_DATA_T1 1
#define MCL_DATA_T2 2
#define MCL_DATA_T3 3
#define MCL_ETX 4
#define MCL_STX 5


#define BUFFER_SIZE 128
volatile int str_p = 0;
char str_buf[BUFFER_SIZE];

int val_i = 0;
int val_p = 0;
int val_buf[BUFFER_SIZE];
int * stx_at = 0;



volatile int start_bit_seen = 0;
volatile int curr_pin_state = 0;
volatile int prev_pin_state = 0;
volatile int prev_pin_state_duration = 1;
volatile int last_decoded_bit = 0;
unsigned long int last_change = 0;

volatile int handling_interrupt = 0;


void setup() {

  // General
  // Serial port
  Serial.begin(230400);
  Serial.println("Data grabber ready");

  // B&O I/O's
  pinMode(dataPin, INPUT);      // Serial data
  last_change = micros();

  // Setup timer and interrupt
  attachInterrupt(digitalPinToInterrupt(dataPin), pinChange, CHANGE);


}

void loop(void) {

}

void dump_message(char * out_buf, int * start, int * end) {
  int * p = start + 1;
  Serial.write("STX ");
  while(p != end) {
    Serial.print(*p, HEX);
    Serial.write(" ");
    p++;
  }

  Serial.write("ETX\n");
  Serial.flush();
}
volatile int processing_message = 0;

void pinChange(void) {
  if (processing_message) {
    Serial.write("OVERFLOW");
  }

  if (digitalRead(dataPin) == HIGH) {
    last_change = micros();
  } else {
    // Beolink signals are spaced by timing units of 3.125 ms.
    // t1 ... t5 is thus 1 ... 5 timeslots; 1 .. 3 is data,
    // 5 is the start bit, 4 is the stop bit.

    int beolink_signal = 1 + ((micros() - last_change) / 3125);

    val_buf[val_p] = beolink_signal;

    switch (beolink_signal) {
      case MCL_STX:
        // Note that a message start was observed at val_buf[val_p].
        stx_at = val_buf + val_p;
        break;
      case MCL_ETX:
        processing_message = 1;
        decode_message(str_buf, stx_at, val_buf + val_p);
        val_p = 0; processing_message = 0;
        break;
      case MCL_DATA_T1:
        if (!last_decoded_bit) {
          Serial.write("Decode error; last_bit = 0 and signal = t1");
        } else {
          val_buf[val_p] = last_decoded_bit = 0;
        }
        break;
      case MCL_DATA_T2:
        val_buf[val_p] = last_decoded_bit = last_decoded_bit ? 1 : 0;
        break;
      case MCL_DATA_T3:
        if (last_decoded_bit) {
          Serial.write("Decode error; last_bit = 1 and signal = t3");
        } else {
          val_buf[val_p] = last_decoded_bit = 1;
        }
        break;
    }

    val_p++;
  }
}
