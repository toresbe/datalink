
#include <TimerOne.h>

// Timing
int MLCPeriod = 1562*2;
//int MLCPeriod = 781;
unsigned long nextMillis = 0;  
unsigned long currMillis = 0;
long periodMillis = 1562*2;

// Digital ports
int dataPin = 2;
int dataState = 0;
int armPin = dataPin;
int armState = 0;
int edgeTrigPin = 4;
int edgeTrigState = 0;

// Data array
const int dataArraySize = 500;
int dataArray[dataArraySize];
const int data2ArraySize = dataArraySize/16;
unsigned data2Array[data2ArraySize];
const int seqDataArraySize = 48;
unsigned seqDataArray[seqDataArraySize];

// Print
String strTime = "";
int timeRes = 10;

//
volatile int g_i = 0;

volatile int tick = 0;

void setup() {
  
  // General
  // Serial port
  Serial.begin(9600);
  Serial.println("Welcome to Joachim's Logic Analyser for B&O DataLink");

  // B&O I/O's
  pinMode(dataPin,INPUT);       // Serial data

  // General I/O's
  pinMode(edgeTrigPin,INPUT);   // Trigger on rising/falling edge

  // Clear data array
  clearDataArray();

  // Setup timer and interrupt
  Timer1.initialize(781);
  Timer1.attachInterrupt(sampleBit);
  
}

void loop() {
  // Wait for arm
  int i = 1;
  armState = digitalRead(armPin);
  
  if (armState == 0) { 

    i = 0;
    dataArray[0] = armState;
    waitTicks(6);
    for (int j = 1; j < dataArraySize; j++) {
      dataArray[j] = readDataBit();
      waitTicks(5);
    }
    
    // Print bits
    printDataArray();
    delay(100); // Get a breah because Serial buffer overflows
    printHex();
    delay(100); // Get a breah because Serial buffer overflows
    
    clearSequence();
    makeSequence();
    printSequence();
    delay(100); // Get a breah because Serial buffer overflows

    clearDataArray(); // Clear data array
  }
}

void clearSequence() {
  for (int i = 0; i < seqDataArraySize; i++) {
    seqDataArray[i] = 0;
  }
}

void makeSequence() {

  // Status
  int k = 0;      // int number
  int state = 0;  // 0: Start recording data byte
                  // 1: Continue recording data byte
                  // 2: Start counting idles
                  // 3: Continue counting idles
  int byteRec = 0;// Recorded temp byte
  int bitCnt = 0; // Counts bits in the temp byte

  int currBit = 0; 
  int nextBit = 0;
  
  // Run through all bits in the wave
  for (int i = 0; i < (dataArraySize - 1); i++) {
    // Break if the squence array overflows!
    if (seqDataArraySize <= k) {
      Serial.println("");
      Serial.println("Input:");
      break;
    }
    // Read bit array
    currBit = dataArray[i] ^ 0x0001;
    nextBit = dataArray[i+1] ^ 0x0001;
    // State machine
    switch (state) {
      case 0:
        byteRec = currBit;
        bitCnt = 0;
        // Next state
        state = 1;
        break;
      case 1:
        byteRec = byteRec << 1;
        byteRec = byteRec | currBit;
        if (bitCnt == 6) {
          seqDataArray[k] = byteRec;
          k = k + 1;
          // Next state
          if (nextBit == 0x0) {   // Read idles
            state = 2;
          } else { // == 0x1      // Read data bytes
            state = 0;
          }
        } else {
          bitCnt = bitCnt + 1;
        }
        break;
      case 2:
        byteRec = 0 | 0x1000;
        if (nextBit == 0x0) {   // Read idles
          state = 3;
        } else { // == 0x1      // Read data bytes
          seqDataArray[k] = byteRec;
          k = k + 1;
          state = 0;
        }
        break;
      case 3:
        byteRec = byteRec + 1;
        if (nextBit == 0x0) {   // Read idles
          state = 3;
        } else { // == 0x1      // Read data bytes
          seqDataArray[k] = byteRec;
          k = k + 1;
          state = 0;
        }
        // Next state
        //state = 0;
        break;
      default: 
        // This state should never be reached
      break;
    }
  }
}

void printSequence() {
  for (int i = 0; i < seqDataArraySize; i++) {
    Serial.print(intToHex(seqDataArray[i]));
    Serial.print(" ");
  }
  Serial.println("");
}

int readDataBit() {
  int dataInput = digitalRead(dataPin);
  //int dataInput = random(0, 2);
  return dataInput;
}

void printHex() {
  Serial.println("");
  int n = 0;
  unsigned int data = 0;
  for (int i = 0; i < data2ArraySize; i++) {
    data = 0;
    for (int j = 0; j < 16; j++) {
      data = data << 1;
      data = data | (dataArray[n] ^ 0x0001); // Note the input is inverted here
      n = n + 1;
    }
    Serial.print(intToHex(data));
    Serial.print(" ");
  }
  Serial.println("");
}


String intToHex(unsigned int k) {
  String res = "0x";
  if        (k >= 0x1000) {
    // do nothing
  } else if (k >= 0x100) {
    res = res + "0";
  } else if (k >= 0x10) {
    res = res + "00";
  } else {
    res = res + "000";
  }
  res = res + String(k, HEX);
  return res;
}

void printDataArray() {
  // Print data wave
  for (int i = 0; i < dataArraySize; i++) {
    if (dataArray[i] == 0) {
      Serial.print("_");
    } else {
      Serial.print("-");
    }
  }
  Serial.println("");

  // Print time
  /*
  int i = 0;
  unsigned long timeIntToStr = 0;
  while (true) {
    // Print value and unit
    strTime = String(timeIntToStr);
    Serial.print(strTime);
    Serial.print("us");
    // Print empty space
    for (int j = strTime.length() + 2; j < timeRes; j++) {
      Serial.print(" ");
    }
    // Is it time to end loop
    i = i + timeRes;
    if (i > dataArraySize) {
      break;
    }
    // What should next time slot show
    timeIntToStr = timeIntToStr + 1200 * timeRes;
  }
  */
  
  Serial.println("");
}

void clearDataArray() {
  for (int i = 0; i < dataArraySize; i++) {
    dataArray[i] = 0;
  }
}

void waitTicks(int n) {
  int tick_copy;
  noInterrupts();
  tick_copy = tick;
  interrupts();
  int next_tick;
  next_tick = tick_copy;
  for (int j = 1; j < n; j++) {
    while (next_tick == tick_copy) {
      noInterrupts();
      tick_copy = tick;
      interrupts();
    }
    next_tick = next_tick ^ 1;
  }
}

void sampleBit(void) {
  tick = tick ^ 1;
}
