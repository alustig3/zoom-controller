# define VERSION 2021010700

#include <Arduino.h>
#include <BleKeyboard.h>
#include <EEPROM.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define OS_BYTE 0
#define MODE_BYTE 1

BleKeyboard bleKeyboard("Zoom Controller","Lustig Labs");

//timing
unsigned long last_active;
unsigned long idle_limit;
const int connection_timer = 30*1000; // shutdown and stop trying to connect after  25 seconds
const int warning_countdown = 10*1000; // send warning 10 seconds before idle shutdown
const int zoom_idle = 30 * 60*1000; //30 minutes
const int youtube_idle = 5 * 60*1000; //5 minutes
const byte debounce = 200;
const int os_hold = 2000;
const int shutdown_hold = 500;

enum os {mac,windows};
enum os active_os;
enum mode {zoom, youtube};
enum mode active_mode;
enum view {speaker,gallery};

void change_mode(mode newmode);
void change_os(os new_os);
void last_chance(int last_chance_timer);
void reset_inactive_clock();

#include "hardware.h"
#include "bluetooth.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(latch_BTN, INPUT_PULLUP);

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector   

  EEPROM.begin(2); 
  active_mode = static_cast<mode>(EEPROM.read(MODE_BYTE));
  if (active_mode != zoom && active_mode != youtube){
    active_mode = zoom;
  }
  active_os = static_cast<os>(EEPROM.read(OS_BYTE));
  if (active_os != mac && active_os != windows){
    active_os = mac;
  }
  
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

  //encoder setup
  pinMode(encoderPin1, INPUT_PULLUP);
  pinMode(encoderPin2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);

  Serial.begin(115200);

  check_for_hold(latch_BTN,5000,print_version);
  while(!digitalRead(latch_BTN)){
    delay(1);
  }
  delay(debounce);
  reset_inactive_clock();
  change_mode(active_mode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  while(!bleKeyboard.isConnected()){
    connecting();
  }
  /***************** ZOOM MODE ********************************/
  if( active_mode == zoom){
    if (tempNow-tempOld>knob_sensitivity){
      bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
      tempNow = tempOld;
      reset_inactive_clock();
    }
    else if (tempNow-tempOld<-knob_sensitivity){
      bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
      tempNow = tempOld;
      reset_inactive_clock();
    }
    else if (!digitalRead(latch_BTN)){
      while(!digitalRead(latch_BTN)){
        if(!digitalRead(btn_1)){
          bool was_held = check_for_hold(btn_1, os_hold, change_os,mac);
          if(!was_held){
            change_mode(zoom);
          }
        }
        else if(!digitalRead(btn_2)){
          bool was_held = check_for_hold(btn_2, os_hold, change_os,windows);
          if(!was_held){
            change_mode(youtube);
          }
        }
      }
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_1)){
      shortcut('a'); //toggle mic
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_2)){
      reset_inactive_clock();
      shortcut('v');//toggle video
    }
    else if (!digitalRead(btn_3)){
      shortcut('s'); //share screen
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_4)){
      reset_inactive_clock();
      if(double_click(btn_4)){
        change_view(gallery);
      }
      else{
        change_view(speaker);
      }
      delay(debounce);
    }
    else if (!digitalRead(btn_5)){
      reset_inactive_clock();
      check_for_hold(btn_5, shutdown_hold, shutdown);
      leave(); 
    }
  }
  /***************** YOUTUBE MODE ********************************/
  else if (active_mode == youtube){
    if (tempNow-tempOld>knob_sensitivity){//CW
      bleKeyboard.write('d');
      tempNow = tempOld;
      reset_inactive_clock();
    }
    else if (tempNow-tempOld<-knob_sensitivity){//CCW
      bleKeyboard.write('s');
      tempNow = tempOld;
      reset_inactive_clock();
    }
    else if (!digitalRead(latch_BTN)){
      bool alternate_cmd = false;
      while(!digitalRead(latch_BTN)){
        if(!digitalRead(btn_1)){
          bool was_held = check_for_hold(btn_1, os_hold, change_os,mac);
          if(!was_held){
            change_mode(zoom);
          }
          alternate_cmd = true;
        }
        else if(!digitalRead(btn_2)){
          bool was_held = check_for_hold(btn_2, os_hold, change_os,windows);
          if(!was_held){
            change_mode(youtube);
          }
          alternate_cmd = true;
        }
      }
      if (!alternate_cmd){ //just clicked, did not click and rotate
          bleKeyboard.write('r');
          delay(debounce);
      }
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_1)){
      while(!digitalRead(btn_1)){
        if (tempNow-tempOld>knob_sensitivity){//CW
          tempNow = tempOld;
          bleKeyboard.write(KEY_MEDIA_VOLUME_UP);
        }
        else if (tempNow-tempOld<-knob_sensitivity){//CCW
          tempNow = tempOld;
          bleKeyboard.write(KEY_MEDIA_VOLUME_DOWN);
        }
      }
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_2)){
      bleKeyboard.write('z'); //seek backward
      pulse();
      delay(debounce);
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_3)){
      bleKeyboard.write('x'); //seek forward
      pulse();
      delay(debounce);
      reset_inactive_clock();
    }
    else if (!digitalRead(btn_4)){
      bleKeyboard.write('f'); //toggle fullscreen
      pulse();
      delay(debounce);
      reset_inactive_clock();
    }

    else if (!digitalRead(btn_5)){
      check_for_hold(btn_5, shutdown_hold, shutdown);
      bleKeyboard.write(' '); //play/pause video
      pulse();
      delay(debounce);
      reset_inactive_clock();
    }
  }

  if (millis() - last_active > (idle_limit - warning_countdown)){
    notify(KEY_F3);//send warning
    last_chance(warning_countdown);
    reset_inactive_clock();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reset_inactive_clock(){
  last_active = millis();
}

void change_mode(mode newmode){
  switch (newmode){
    case zoom:
      notify(KEY_F4); 
      idle_limit = zoom_idle;
      break;
    case youtube:
      notify(KEY_F5); 
      idle_limit = youtube_idle;
      break;
  }
  pulse();
  active_mode = newmode;
}

void change_os(os new_os){
  if (new_os == windows){
    active_os = windows;
    notify(KEY_F2);
    blink_yellow(7);
  }
  else if (new_os == mac){
    active_os = mac;
    notify(KEY_F1);
    blink_yellow(3);
  }
}

void last_chance(int last_chance_timer){
  unsigned long start_last_chance = millis();
  int blink_delay = 150;
  bool stay_awake = false;
  while( millis() - start_last_chance < last_chance_timer){
    for( int i = 0 ; i<blink_delay; i++){
      ledcWrite(greenPWM,0);
      if(check_for_activity()){
        stay_awake = true;
        start_last_chance = millis() - last_chance_timer;
      }
      delay(1);
    }
    for( int i = 0 ; i<blink_delay; i++){
      ledcWrite(greenPWM,255);
      if(check_for_activity()){
        stay_awake = true;
        start_last_chance = millis() - last_chance_timer;
      }
      delay(1);
    }
  }
  if(stay_awake){
    ledcWrite(greenPWM,knob_brightness);
    delay(debounce);
  }
  else{
    shutdown();
  }
}