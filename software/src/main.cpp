#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Zoom Pad_new","Lustig Labs");

// const byte encoder_BTN = 25;
const byte latch_BTN = 25;
const byte encoderPin1 = 22;
const byte encoderPin2 = 21;
volatile int lastEncoded = 0;
int tempNow=0,tempOld = 0;
const byte knob_sensitivity = 2;
const byte red_LED = 26;
const byte green_LED = 27;
const int redPWM = 0;
const int greenPWM = 1;
const int freq = 500; // 5000 Khz produced a hum at 50% duty cycle, so brought this down
const int resolution = 8;
int level = 0;
const byte btn_4 = 16;
const byte btn_3 = 17;
const byte btn_2 = 18;
const byte btn_1 = 19;
const int hold_time = 2000;
const byte max_bright = 75;

bool isMAC = true;
const byte hack = 14;

unsigned long last_active;
const long idle_limit = 60 * 1000*60; // turn off after 60 minutes of inactivity

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
  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);
}

void shortcut(char ltr){
  if (isMAC){
    bleKeyboard.press(KEY_LEFT_SHIFT);
    bleKeyboard.press(KEY_LEFT_GUI);
  }
  else{
    bleKeyboard.press(KEY_LEFT_ALT);
  }
    bleKeyboard.press(ltr);
    delay(250);
    bleKeyboard.releaseAll();
}

void shutdown(){
  ledcWrite(greenPWM,0);
  ledcWrite(redPWM,0);
  pinMode(latch_BTN,OUTPUT);
  digitalWrite(latch_BTN,LOW);
}

void reset_inactive_clock(){
  last_active = millis();
}

void setup() {
  pinMode(hack, OUTPUT);
  digitalWrite(hack,HIGH);
  pinMode(latch_BTN, INPUT_PULLUP);
  ledcSetup(redPWM, freq, resolution);
  ledcSetup(greenPWM, freq, resolution);
  ledcWrite(greenPWM,level);
  ledcAttachPin(red_LED, redPWM);
  ledcAttachPin(green_LED, greenPWM);
  pinMode(btn_1,INPUT_PULLUP);
  pinMode(btn_2,INPUT_PULLUP);
  pinMode(btn_4,INPUT_PULLUP);
  pinMode(btn_3,INPUT_PULLUP);
  Serial.begin(115200);
  encoderSetup();
  bleKeyboard.begin();
  reset_inactive_clock();
}

void loop() {
  ledcWrite(redPWM,level);
  ledcWrite(greenPWM,max_bright-level);
  if (tempNow-tempOld>knob_sensitivity){ //CCW knob turn
    Serial.println("CW");
    bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
    level+=10;
    if (level>max_bright){
      level = max_bright;
    }
    tempNow = tempOld;
    digitalWrite(red_LED,HIGH);
    reset_inactive_clock();
  }
  else if (tempNow-tempOld<-knob_sensitivity){ //CW knob turn
    Serial.println("CCW");
    level-=10;
    if (level<0){
      level = 0;
    }
    bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
    tempNow = tempOld;
    reset_inactive_clock();
  }
  else if (!digitalRead(latch_BTN)){
    Serial.println("Encoder Press!");
    unsigned long presstime = millis();
    while(!digitalRead(latch_BTN)){
      if(millis() - presstime > hold_time){
        shutdown();
      }
    }
    if( millis() - presstime < 200){
      delay(200);
    }
    shortcut('H');
    reset_inactive_clock();
  }
  else if (!digitalRead((btn_1))){
    Serial.println("YEllow Pressed!");
    delay(200);
    shortcut('A');
    reset_inactive_clock();
  }
  else if (!digitalRead((btn_2))){
    Serial.println("Green Pressed!");
    delay(200);
    shortcut('V');
    reset_inactive_clock();
  }
  else if (!digitalRead((btn_3))){
    Serial.println("BLUE Pressed!");
    delay(200);
    shortcut('S');
    reset_inactive_clock();
  }
  else if (!digitalRead((btn_4))){
    Serial.println("Red Pressed!");
    delay(200);
    shortcut('W');
    reset_inactive_clock();
  }
  else if (millis() - last_active > idle_limit){
    shutdown();
  }

}

