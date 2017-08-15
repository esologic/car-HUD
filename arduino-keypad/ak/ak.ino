#define MODECOMBO 0
#define MODESTART 1


#define number_of_74hc595s 2 //How many of the shift registers - change this
#define numOfRegisterPins number_of_74hc595s * 8
int SER_PIN = 8;   //pin 14 on the 75HC595
int RCLK_PIN = 9;  //pin 12 on the 75HC595
int SRCLK_PIN = 10; //pin 11 on the 75HC595
boolean registers[numOfRegisterPins];

unsigned long lastBounceTime = 0;
unsigned long lastPressTime = 0;
unsigned long lastTransitionTime = 0;

const long debounceInterval = 10;
const long timeoutInterval = 3000;
const long transitionInterval = 5000;

#define NUMBUTTONS 3
int buttons[NUMBUTTONS] = {2, 3, 4};
int lastStates[NUMBUTTONS];

#define COMBOLENGTH 4
int comboPos = 0;
int comboBuffer[COMBOLENGTH];
static int goodCombo[COMBOLENGTH] = {1, 2, 1, 0};

int state; 

void setup() {

  Serial.begin(9600);

  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);

  for (int i; i < NUMBUTTONS; i++) {
    pinMode(buttons[i], INPUT);
    lastStates[i] = 0;
  }

  pinMode(SER_PIN, OUTPUT);
  pinMode(RCLK_PIN, OUTPUT);
  pinMode(SRCLK_PIN, OUTPUT);

  //reset all register pins
  clearRegisters();
  writeRegisters();

  state = 0; 
  
}

void loop() {

  unsigned long currentMillis = millis();

  switch (state) {
    
    case MODECOMBO:
      Serial.println("State 0");
      setRegisterPin(9, LOW);
      setRegisterPin(10, LOW);
      setRegisterPin(11, HIGH);
      writeRegisters();
      
      if (currentMillis - lastBounceTime >= debounceInterval) {
    
        lastBounceTime = currentMillis;
      
        for (int buttonNumber; buttonNumber < NUMBUTTONS; buttonNumber++) {
      
          int buttonState = digitalRead(buttons[buttonNumber]);
      
          // this ensures that the button is released before registering new presses
          if ((buttonState != lastStates[buttonNumber]) && (buttonState)) {
            lastPressTime = currentMillis;
            if (addPress(buttonNumber)) {
              state = MODESTART;
              lastTransitionTime = currentMillis;
            }
          }
          lastStates[buttonNumber] = buttonState;
        }
      }

      if ((currentMillis - lastPressTime >= timeoutInterval) &&  (comboPos > 0)){
        Serial.println("timeout!");
        restartCombo();
      }
      
      break;

    case MODESTART:
      Serial.println("State 1");
      setRegisterPin(9, LOW);
      setRegisterPin(10, HIGH);
      setRegisterPin(11, LOW);
      writeRegisters();

      // the machine will transition back into 
      if (currentMillis - lastTransitionTime >= transitionInterval) {
        state = 0;
      }
            
      break;
  }
}

void restartCombo(void) {
  comboPos = 0;
  for (int i; i < NUMBUTTONS; i++) {
    setRegisterPin(i, LOW);
  }
  writeRegisters();
}

bool addPress(int buttonNumber) {
  
  setAndWrite(buttonNumber, HIGH);
  
  comboBuffer[comboPos] = buttonNumber;
  comboPos++;

  if (comboPos == COMBOLENGTH) {
    
    for (int i; i < COMBOLENGTH; i++) {

      bool correct = comboBuffer[i] == goodCombo[i];

      if (!correct) {
        restartCombo();
        return false;
      }
    }
    return true;
  }
  return false;
}

void setAndWrite(int index, int value) {
  setRegisterPin(index, value);
  writeRegisters();
}

void clearRegisters(){
  for(int i = numOfRegisterPins - 1; i >=  0; i--){
     registers[i] = LOW;
  }
} 

void writeRegisters() {

  digitalWrite(RCLK_PIN, LOW);

  for (int i = numOfRegisterPins - 1; i >=  0; i--){
    
    digitalWrite(SRCLK_PIN, LOW);

    int val = registers[i];

    digitalWrite(SER_PIN, val);
    digitalWrite(SRCLK_PIN, HIGH);
  }
  digitalWrite(RCLK_PIN, HIGH);
}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value){
  registers[index] = value;
}

