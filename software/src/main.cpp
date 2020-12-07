#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Zoom Pad","Lustig Labs");

const byte encoder_BTN = 15;
const byte encoderPin1 = 33;
const byte encoderPin2 = 27;
volatile int lastEncoded = 0;
int tempNow=0,tempOld = 0;
const byte knob_sensitivity = 2;
const byte red_LED = 32;
const byte green_LED = 14;
const int redPWM = 0;
const int greenPWM = 1;
const int freq = 500; // 5000 Khz produced a hum at 50% duty cycle, so brought this down
const int resolution = 8;
int level = 0;

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
  ledcSetup(redPWM, freq, resolution);
  ledcSetup(greenPWM, freq, resolution);
  ledcAttachPin(red_LED, redPWM);
  ledcAttachPin(green_LED, greenPWM);
}

void loop() {
  ledcWrite(greenPWM,level);
  ledcWrite(redPWM,255-level);

  if (tempNow-tempOld>knob_sensitivity){ //CCW knob turn
    Serial.println("CW");
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    level+=10;
    if (level>255){
      level = 255;
    }
    tempNow = tempOld;
    digitalWrite(red_LED,HIGH);
  }
  else if (tempNow-tempOld<-knob_sensitivity){ //CW knob turn
    Serial.println("CCW");
    level-=10;
    if (level<0){
      level = 0;
    }
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    tempNow = tempOld;
  }
  if (!digitalRead(encoder_BTN)){
    Serial.println("Button Press!");
    digitalWrite(red_LED,LOW);
    delay(250);
  }
}
