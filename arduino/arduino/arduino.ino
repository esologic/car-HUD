/* Start of error codes */
#define NUMERRORCODES 1
#define CRCFAIL 0

#define MESSAGESIZE 3 // the number of bytes in a message

#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX

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

#define NUMDIGITALINPUTPINS 1
int digitalInputPins[NUMDIGITALINPUTPINS] = {4};

#define NUMINPUTPINS NUMANALOGINPUTPINS + NUMDIGITALINPUTPINS
int inputPins[NUMINPUTPINS]; // will hold analogInputPins + digitalInputPins

int (*readPtr[NUMINPUTPINS])(uint8_t); // will hold the read (digitalRead or analogRead or something else) function for a given pin

message errorMessages[NUMERRORCODES]; // will hold all of the error messages unions

void setup() {

  Serial.begin(9600);
  rpiSerial.begin(9600);

  // combine the two arrays of input pins
  memcpy(inputPins, analogInputPins, NUMANALOGINPUTPINS * sizeof(int));
  memcpy(inputPins + NUMANALOGINPUTPINS, digitalInputPins, NUMDIGITALINPUTPINS * sizeof(int));

  // set the read functions in readPtr
  for (int index = 0; index < NUMANALOGINPUTPINS; index++){ readPtr[index] = analogRead; };
  for (int index = 0; index < NUMDIGITALINPUTPINS; index++){ pinMode(digitalInputPins[index], INPUT); readPtr[index + NUMANALOGINPUTPINS] = digitalRead; };

  errorMessages[CRCFAIL] = {0x00, 0xFF, 0xFF};

  printMsg(errorMessages[CRCFAIL]);
  
}

void loop() {
    
  if (rpiSerial.available() >= MESSAGESIZE) { // wait for a full MESSAGE to arrive
    
    message masterMessage = readMsg(rpiSerial);
    
    byte incomingMasterCRC = calcCRC(masterMessage.dataBytes, MESSAGESIZE-1);

    if (incomingMasterCRC == masterMessage.CRC) { // if the CRC from the master passes
      
      intBytes reading;
      
      switch (masterMessage.M0) {
        case 0: // read mode -> Send values to master
          reading.i = readPtr[masterMessage.M1](inputPins[masterMessage.M1]);
          break;
        case 1: // write mode -> Set values on arduino
          break;
      }

      message slaveMessage;
      memcpy(slaveMessage.dataBytes, reading.bytes, MESSAGESIZE-1);
      slaveMessage.CRC = calcCRC(slaveMessage.dataBytes, MESSAGESIZE-1); 
  
      writeMsg(rpiSerial, slaveMessage);
            
    } else { // Bad master CRC
      Serial.println("Sending CRC!");
      writeMsg(rpiSerial, errorMessages[CRCFAIL]);
    }
  }
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
