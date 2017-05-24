#define MESSAGESIZE 3
#include <SoftwareSerial.h>

SoftwareSerial rpiSerial(3, 2); // RX, TX


int input_0 = 0;
int input_1 = 1;

union intBytes{
  int i;
  byte bytes[2];
};

void setup() {
  Serial.begin(9600);
  rpiSerial.begin(9600);
}

void loop() {

  byte messageBuff[MESSAGESIZE]; 
  
  if (rpiSerial.available())
  {
    rpiSerial.readBytes(messageBuff, MESSAGESIZE);

    intBytes reading;
    
    reading.i = analogRead(input_1);

    byte CRC = intBytesCRC(reading);

    rpiSerial.write(reading.bytes[0]);
    rpiSerial.write(reading.bytes[1]);    
    rpiSerial.write(CRC);

    Serial.print("Byte1: ");
    Serial.print(reading.bytes[0], HEX);
    Serial.print(" Byte2: ");
    Serial.print(reading.bytes[1], HEX); 
    Serial.print(" Byte3: "); 
    Serial.print(CRC, HEX);
    Serial.print(" ");
    Serial.print(reading.i, DEC);
    Serial.print(" ");
    Serial.print(CRC, DEC);
    Serial.println("");
    
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

