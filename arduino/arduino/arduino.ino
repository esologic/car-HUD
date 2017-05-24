#define MESSAGESIZE 3
#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX

int input_0 = 0;
int input_1 = 1;

union message {
    byte bytes[MESSAGESIZE];
    struct {
        byte M0;
    };
    struct {
        byte unused0; // for offsetting DO NOT USE
        byte M1;
    };
    struct {
        byte unused1; // for offsetting DO NOT USE
        byte unused2; // for offsetting DO NOT USE
        byte CRC;
    };
};

union intBytes {
  int i;
  byte bytes[2];
};


void setup() {
  Serial.begin(9600);
  rpiSerial.begin(9600);
}

void loop() {

  byte messageBuff[MESSAGESIZE]; 
  
  if (rpiSerial.available() >= MESSAGESIZE)
  {

    message masterMessage;
    
    rpiSerial.readBytes(masterMessage.bytes, MESSAGESIZE);
        
    Serial.print("M0: ");
    Serial.print(masterMessage.M0); 
    Serial.print(" M1: ");
    Serial.print(masterMessage.M1);
    Serial.print(" CRC: ");
    Serial.print(masterMessage.CRC);  
    Serial.println(" ");
    
    intBytes reading;
    
    switch (masterMessage.M0)
    {
      case 0:
 
        switch (masterMessage.M1)
        {
          case 0:
            reading.i = analogRead(input_0);
            break;
          case 1:
            reading.i = analogRead(input_1);
            break;
        }
        
        break;
    }
    
    byte CRC = intBytesCRC(reading);

    rpiSerial.write(reading.bytes[0]);
    rpiSerial.write(reading.bytes[1]);    
    rpiSerial.write(CRC);    
  }
}

byte intBytesCRC(intBytes ib)
{
  byte b = ib.bytes[0] + ib.bytes[1];
  return b;
}

void debugInputs()
{
  int pins[2] = {input_0, input_1}; 

  for (int index = 0; index < 2; index++)
  {
    intBytes reading;
    
    reading.i = analogRead(pins[index]);

    Serial.print("Reading ");
    Serial.print(index);
    Serial.print(" Int [");
    Serial.print(reading.i);
    Serial.print("] Byte 0 [");
    Serial.print(reading.bytes[0], HEX);
    Serial.print("] Byte 1 [");
    Serial.print(reading.bytes[1], HEX);
    Serial.print("] ");
  }
  
  Serial.println("");
  
}

