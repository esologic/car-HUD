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
  for (byte index = 0; index < 255; index++)
  {
    rpiSerial.write(index);
    Serial.print(index, HEX);
    Serial.println("");
  }
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

