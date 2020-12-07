#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Zoom Pad","Lustig Labs");

const byte encoder_BTN = 15;
const byte encoderPin1 = 33;
const byte encoderPin2 = 27;
volatile int lastEncoded = 0;
int tempNow=0,tempOld = 0;
const byte knob_sensitivity = 2;

void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) tempNow++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) tempNow--;
  lastEncoded = encoded; //store this value for next time
}

void encoderSetup(){
  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);
  pinMode(encoder_BTN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);
}

void setup() {
  Serial.begin(115200);
  encoderSetup();
  bleKeyboard.begin();
}

void loop() {
  if (tempNow-tempOld>knob_sensitivity){ //CCW knob turn
    Serial.println("CW");
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    tempNow = tempOld;
  }
  else if (tempNow-tempOld<-knob_sensitivity){ //CW knob turn
    Serial.println("CCW");
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    tempNow = tempOld;
  }
  if (!digitalRead(encoder_BTN)){
    Serial.println("Button Press!");
    delay(250);
  }
}
