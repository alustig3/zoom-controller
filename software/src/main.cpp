#include <Arduino.h>
#include <BleKeyboard.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BleKeyboard bleKeyboard("Zoom Controller","Lustig Labs");

const byte latch_BTN = 25;
const byte encoderPin1 = 22;
const byte encoderPin2 = 23;
volatile int lastEncoded = 0;
int tempNow=0,tempOld = 0;
const byte knob_sensitivity = 2;
const byte red_LED = 26;
const byte green_LED = 27;
const int redPWM = 0;
const int greenPWM = 1;
const int freq = 500; // 5000 Khz produced a hum at 50% duty cycle, so brought this down
const int resolution = 8;
const byte btn_1 = 19;
const byte btn_2 = 18;
const byte btn_3 = 17;
const byte btn_4 = 16;
const byte btn_5 = 21;
const int hold_time = 850;
byte knob_brightness = 100;
const int button_bounce = 200;

bool isMAC = true;

unsigned long last_active;
const long idle_limit = 5 * 1000*60; // turn off after 5 minutes of inactivity
const int warning_countdown = 10*1000; // send warning 10 seconds before idle shutdown

enum mode {zoom, youtube};
enum mode active_mode = zoom;

bool warning_sent = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

void pulse(){
  for (int i = 255; i >knob_brightness; i--){
    ledcWrite(greenPWM,i);
    delay(1);
  }
}

void zoom_mode(){
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(KEY_F1);
  bleKeyboard.releaseAll();
  pulse();
  active_mode = zoom;
}

void youtube_mode(){
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(KEY_F2);
  bleKeyboard.releaseAll();
  pulse();
  active_mode = youtube;
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
  bleKeyboard.releaseAll();
  pulse();
  delay(50);
}

void leave(){
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press('w');
  bleKeyboard.releaseAll();
  for (int i = 255; i >knob_brightness; i--){
    ledcWrite(greenPWM,i);
    delay(1);
  }
  delay(50);
}

void shutdown(){
  ledcWrite(greenPWM,0);
  ledcWrite(redPWM,0);
  pinMode(latch_BTN,OUTPUT);
  digitalWrite(latch_BTN,LOW);
}

void reset_inactive_clock(){
  last_active = millis();
  warning_sent = false;
}

bool double_click( int btn_pin){
  while(!digitalRead(btn_pin)){
    delay(2); //in first press
  }
  int second_press_timer = 0;
  bool double_clicked = false;
  while(second_press_timer < 120){
    delay(1);
    second_press_timer++;
    if (!digitalRead(btn_pin)){
      Serial.println("Double click!");
      double_clicked = true;
      break;
    }
  }
  Serial.println(second_press_timer);
  return double_clicked;
}

void connecting(){
  last_active = millis();
  ledcWrite(greenPWM,0);
  bool going_up = true;
  byte red_brightness = knob_brightness;
  while(!bleKeyboard.isConnected()){
    delay(2);
    if (!digitalRead(latch_BTN)){
      if(double_click(latch_BTN)){
        shutdown();
      }
    }
    if (going_up){
      if (red_brightness<255){
        red_brightness++;
        ledcWrite(redPWM,red_brightness);
      }
      else{
        going_up = false;
      }
    }
    else{
      if (red_brightness>0){
        red_brightness--;
        ledcWrite(redPWM,red_brightness);
      }
      else{
        going_up = true;
      }
    }
    if (millis() - last_active > 2000){// shutdown and stop trying to connect after 20 seconds
      shutdown();
    }
  }
  delay(200);
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press(KEY_LEFT_ALT);
  if(active_mode==zoom){
    bleKeyboard.press(KEY_F1);
  }
  else{
    bleKeyboard.press(KEY_F2);
  }
  bleKeyboard.releaseAll();
  ledcWrite(redPWM,0);
  ledcWrite(greenPWM,knob_brightness);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector   
  pinMode(latch_BTN, INPUT_PULLUP);
  ledcSetup(redPWM, freq, resolution);
  ledcWrite(redPWM,knob_brightness);
  ledcSetup(greenPWM, freq, resolution);
  ledcAttachPin(red_LED, redPWM);
  ledcAttachPin(green_LED, greenPWM);
  pinMode(btn_1,INPUT_PULLUP);
  pinMode(btn_2,INPUT_PULLUP);
  pinMode(btn_3,INPUT_PULLUP);
  pinMode(btn_4,INPUT_PULLUP);
  pinMode(btn_5,INPUT_PULLUP);
  bleKeyboard.begin();
  connecting();
  encoderSetup();
  Serial.begin(115200);
  reset_inactive_clock();
  while(!digitalRead(latch_BTN)){
    delay(1);
  }
}

void loop() {
  while(!bleKeyboard.isConnected()){
    connecting();
  }

  ///////////// CCW Knob Turn //////////////////
  if (tempNow-tempOld>knob_sensitivity){
    Serial.println("CW");
    tempNow = tempOld;
    reset_inactive_clock();

    switch(active_mode){
      case zoom:
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        break;
      case youtube:
        bleKeyboard.write('d');
        break;
    }
  }

  ///////// CW Knob Turn ///////////////////////
  else if (tempNow-tempOld<-knob_sensitivity){
    Serial.println("CCW");
    tempNow = tempOld;
    reset_inactive_clock();

    switch(active_mode){
      case zoom:
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        break;
      case youtube:
        bleKeyboard.write('s');
        break;
    }
  }

  /////////// Encoder Button Press ////////////
  else if (!digitalRead(latch_BTN)){
    reset_inactive_clock();

    if(double_click(latch_BTN)){
      shutdown();
    }

    if (active_mode == youtube){
        bleKeyboard.write('r');
        delay(button_bounce);
    }
  }

  //////////////  Button 1 ////////////////////
  else if (!digitalRead(btn_1)){
    Serial.println("Button 1");
    reset_inactive_clock();

    if (active_mode == youtube){
      while(!digitalRead(btn_1)){
        if (tempNow-tempOld>knob_sensitivity){
          Serial.println("CW");
          tempNow = tempOld;
          reset_inactive_clock();
          bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        }
        else if (tempNow-tempOld<-knob_sensitivity){
          Serial.println("CCW");
          tempNow = tempOld;
          reset_inactive_clock();
          bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        }
      }
    }
    else{
      shortcut('A');
    }
  }

  //////////////  Button 2 ////////////////////
  else if (!digitalRead(btn_2)){
    Serial.println("Button 2");
    reset_inactive_clock();

    if(double_click(btn_2)){
      if (active_mode == zoom){
        youtube_mode();
      }
      else{
        zoom_mode();
      }
    }
    else{
      if (active_mode==zoom){
        shortcut('V');
      }
    }
  }

  //////////////  Button 3 ////////////////////
  else if (!digitalRead(btn_3)){
    Serial.println("Button 3");
    reset_inactive_clock();

    if(active_mode==zoom){
      shortcut('S');
    }
    else if (active_mode == youtube){
      bleKeyboard.write('z');
      pulse();
      delay(button_bounce);
    }
  }

  //////////////  Button 4 ////////////////////
  else if (!digitalRead(btn_4)){
    Serial.println("Button 4");
    reset_inactive_clock();

    if(active_mode == zoom){
      shortcut('W');
    }
    else if (active_mode == youtube){
      bleKeyboard.write('x');
      pulse();
      delay(button_bounce);
    }
  }

  //////////////  Button 5 ////////////////////
  else if (!digitalRead(btn_5)){
    Serial.println("Button 5");
    reset_inactive_clock();

    if(active_mode == zoom){
      leave();
    }
    else if (active_mode == youtube){
      bleKeyboard.write(' ');
      pulse();
      delay(button_bounce);
    }
  }

  else if (millis() - last_active > (idle_limit - warning_countdown)  && !warning_sent){
    //send warning
    bleKeyboard.press(KEY_LEFT_CTRL);
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press(KEY_F3);
    bleKeyboard.releaseAll();
    warning_sent = true;
  }
  //////////////  Shutdown if idle ////////////////////
  else if (millis() - last_active > idle_limit){
    shutdown();
  }
}