/* Start of error codes */
#define NUMERRORCODES 2
#define READY 0
#define CRCFAIL 1

#define MESSAGESIZE 3 // the number of bytes in a message

#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(9, 8); // RX, TX

union message {
    byte rawBytes[MESSAGESIZE]; // holds the two data bytes and the CRC
    byte dataBytes[MESSAGESIZE-1]; // hold only the data bytes, without the CRC
    struct {
        byte M0; // holds the first data byte
    };
    struct {
        byte unused0; // for offsetting DO NOT USE
        byte M1; // holds the second data byte
    };
    struct {
        byte unused1; // for offsetting DO NOT USE
        byte unused2; // for offsetting DO NOT USE
        byte CRC; // holds the checksum
    };
};

union intBytes {
  int i;
  byte bytes[2];
};

#define NUMANALOGINPUTPINS 6
int analogInputPins[NUMANALOGINPUTPINS] = {0, 1, 2, 3, 4, 5};

#define NUMDIGITALINPUTPINS 2
int digitalInputPins[NUMDIGITALINPUTPINS] = {4, 5};

#define NUMINPUTPINS NUMANALOGINPUTPINS + NUMDIGITALINPUTPINS
int inputPins[NUMINPUTPINS]; // will hold analogInputPins + digitalInputPins

int (*readPtr[NUMINPUTPINS])(uint8_t); // will hold the read (digitalRead or analogRead or something else) function for a given pin

message errorMessages[NUMERRORCODES]; // will hold all of the error messages unions

int freqPin = 2;
volatile int pushCount = 0;
volatile unsigned long previousInterruptTime = 0;
volatile unsigned long previousPulseTime = 0;
#define NUMPULSES 3
volatile unsigned long pulseTimes[NUMPULSES];
int pulseTimeInsertPosition = 0;
volatile int fullness = 0;
int ave = 0;


void setup() {

  Serial.begin(9600);
  rpiSerial.begin(9600);

  // combine the two arrays of input pins
  memcpy(inputPins, analogInputPins, NUMANALOGINPUTPINS * sizeof(int));
  memcpy(inputPins + NUMANALOGINPUTPINS, digitalInputPins, NUMDIGITALINPUTPINS * sizeof(int));

  // set the read functions in readPtr
  for (int index = 0; index < NUMANALOGINPUTPINS; index++){ readPtr[index] = analogRead; };
  for (int index = 0; index < NUMDIGITALINPUTPINS; index++){ pinMode(digitalInputPins[index], INPUT); readPtr[index + NUMANALOGINPUTPINS] = digitalRead; };

  // populate the errorMessages array
  errorMessages[CRCFAIL] = {0x00, 0xFF, 0xFF};
  errorMessages[READY] = {0x00, 0xFE, 0xFE};

  pinMode(freqPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(freqPin), newPulse, RISING);

  Serial.println("reset");
}


void loop() {
  tryProcessMessage(rpiSerial, true); // try and process a message on the 

  unsigned long sum = 0;
  int c = 0;

  for (int index = 0; index < fullness; index++) {
    unsigned long t = pulseTimes[index];
    sum += t;
    c++;
  }
    
  if (fullness > 0) {
    ave = (sum/c);
  }
}


bool tryProcessMessage(SoftwareSerial &port, bool debugMode) {
  
  if (port.available() >= MESSAGESIZE) { // wait until a full MESSAGE to arrive
    
    message masterMessage = readMsg(port);

    if (debugMode) {
      Serial.print("Got message from master: ");
      printMsg(masterMessage);
    }
    
    byte incomingMasterCRC = calcCRC(masterMessage.dataBytes, MESSAGESIZE-1);

    if (incomingMasterCRC == masterMessage.CRC) { // if the CRC from the master passes
      
      intBytes reading; // used to convert the int to a byte array for transmission
      
      switch (masterMessage.M0) {
        case 0: // read mode -> Send values to master
          if ((masterMessage.M1 >= 0) && (masterMessage.M1 < NUMINPUTPINS)) {
            reading.i = readPtr[masterMessage.M1](inputPins[masterMessage.M1]);
          } else {
            reading.i = ave;
          }
            
          break;
        case 1: // write mode -> Set values on arduino
          break;
        case 2:
          if (debugMode) {
            Serial.println("Sending Ready");
          }
          writeMsg(port, errorMessages[READY]);
          return true;
          break;
        default:
          if (debugMode) {
            Serial.println("Sending CRC Error - But CRC passed");
          }
          writeMsg(port, errorMessages[CRCFAIL]);
          break;
      }

      message slaveMessage;
      memcpy(slaveMessage.dataBytes, reading.bytes, MESSAGESIZE-1);
      slaveMessage.CRC = calcCRC(slaveMessage.dataBytes, MESSAGESIZE-1);

      writeMsg(port, slaveMessage);
      
      if (debugMode) {
        Serial.println("Sending Regular Message");
        Serial.print("Reading: ");
        Serial.println(reading.i);
        printMsg(slaveMessage);
      }
            
    } else { // Bad master CRC
      
      if (debugMode) {
        Serial.print("Bad Master Message: ");
        printMsg(masterMessage);
      }
      
      drainBuffer(port);
      writeMsg(port, errorMessages[CRCFAIL]);
    }
    return true;
  }
  return false; 
}

void drainBuffer(SoftwareSerial &port) {
  while (port.available()) {
    port.read();
  }
}

bool compareByteArrays(byte array1[], byte array2[], int arrayLength) {
  for (int index = 0; index < arrayLength; index++) {
    if (array1[index] != array2[index]) {
      return false;
    }
  }
  return true;
}

message readMsg(SoftwareSerial &port) {
  message incomingMessage;
  port.readBytes(incomingMessage.rawBytes, MESSAGESIZE); // read the raw bytes from the serial port into a message struct
  return incomingMessage;
}

void writeMsg(SoftwareSerial &port, message m) {
  for (int index = 0; index < MESSAGESIZE; index++) {
    port.write(m.rawBytes[index]);
  }
}

void printMsg(message m) {
  Serial.print("Message [");
  Serial.print("M0: ");
  Serial.print(m.rawBytes[0], HEX);
  Serial.print(" M1: ");
  Serial.print(m.rawBytes[1], HEX);
  Serial.print(" CRC: ");
  Serial.print(m.rawBytes[2], HEX);
  Serial.println("]");
}

byte calcCRC(byte byteArray[], int numBytes) {
  byte b = 0;
  for (int index = 0; index < numBytes; index++)
  {
    b = b + byteArray[index];
  }
  return b;
}

void calculatePulseTime() {
  unsigned long pulseTime = millis();
  unsigned long delta = pulseTime - previousPulseTime;
  pulseTimes[pulseTimeInsertPosition] = delta;

  pulseTimeInsertPosition++;
  if (pulseTimeInsertPosition >= NUMPULSES) {
    pulseTimeInsertPosition = 0;
  }

  if (fullness < NUMPULSES) {
    fullness++;
  }
  
  previousPulseTime = pulseTime;
}

/* The following functions are Interrupt Service Routines */ 

void newPulse(void) { // this is to debounce the pulses, may not be essential
  unsigned long interruptTime = millis();
  if (interruptTime - previousInterruptTime > 10) {
     calculatePulseTime();
  }
  previousInterruptTime = interruptTime;
}
