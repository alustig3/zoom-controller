#include <Arduino.h>
#include <BleKeyboard.h>

BleKeyboard bleKeyboard("Hello Andy","Lustig Labs");

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
byte knob_brightness = 25;
const int button_bounce = 200;

bool isMAC = true;

unsigned long last_active;
const long idle_limit = 20 * 1000*60; // turn off after 20 minutes of inactivity

enum mode {zoom, youtube};
enum mode active_mode;

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
  bleKeyboard.releaseAll();
  for (int i = 255; i >knob_brightness; i--){
    ledcWrite(greenPWM,i);
    delay(1);
  }
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
}

bool double_click( int btn_pin){
  while(!digitalRead(btn_pin)){
    delay(2); //in first press
  }
  int second_press_timer = 0;
  bool double_clicked = false;
  while(second_press_timer < 200){
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
  }
  ledcWrite(redPWM,0);
  ledcWrite(greenPWM,knob_brightness);
}

void setup() {
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
  if (!digitalRead(btn_1)){
    active_mode = youtube;
    ledcWrite(redPWM,10);
  }
  else{
    active_mode = zoom;
  }
  while(!digitalRead(latch_BTN) | !digitalRead(btn_1)){
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

///////////// Encoder Button Press /////////////////
  else if (!digitalRead(latch_BTN)){
    if(double_click(latch_BTN)){
      shutdown();
    }

    if (active_mode == youtube){
        bleKeyboard.write('r');
        delay(button_bounce);
    }

    reset_inactive_clock();
  }

  //////////////  Button 1 ////////////////////
  else if (!digitalRead((btn_1))){
    Serial.println("Button 1");
    reset_inactive_clock();

    switch(active_mode){
      case zoom:
        shortcut('A');
        break;
      case youtube:
        bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        delay(button_bounce);
        break;
    }
  }

  //////////////  Button 2 ////////////////////
  else if (!digitalRead((btn_2))){
    Serial.println("Button 2");
    reset_inactive_clock();

    switch(active_mode){
      case zoom:
        shortcut('V');
        break;
      case youtube:
        bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        delay(button_bounce);
        break;
    }
  }

  //////////////  Button 3 ////////////////////
  else if (!digitalRead((btn_3))){
    Serial.println("Button 3");
    reset_inactive_clock();

    if(active_mode==zoom){
      shortcut('S');
    }
    else if (active_mode == youtube){
      bleKeyboard.write('z');
      delay(button_bounce);
    }
  }

  //////////////  Button 4 ////////////////////
  else if (!digitalRead((btn_4))){
    Serial.println("Button 4");
    reset_inactive_clock();

    if(active_mode == zoom){
      shortcut('W');
    }
    else if (active_mode == youtube){
      bleKeyboard.write('x');
      delay(button_bounce);
    }
  }

  //////////////  Button 5 ////////////////////
  else if (!digitalRead((btn_5))){
    Serial.println("Button 5");
    reset_inactive_clock();

    if(active_mode == zoom){
      leave();
    }
    else if (active_mode == youtube){
      bleKeyboard.write(' ');
      delay(button_bounce);
    }
  }

  //////////////  Shutdown if idle ////////////////////
  else if (millis() - last_active > idle_limit){
    shutdown();
  }
}