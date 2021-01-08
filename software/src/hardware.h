// pin defines
const byte latch_BTN = 25;
const byte encoderPin1 = 22;
const byte encoderPin2 = 23;
const byte btn_1 = 19;
const byte btn_2 = 18;
const byte btn_3 = 17;
const byte btn_4 = 16;
const byte btn_5 = 21;
const byte red_LED = 26;
const byte green_LED = 27;

//knob
volatile int lastEncoded = 0;
int tempNow = 0,tempOld = 0;
const byte knob_sensitivity = 2;
const byte resolution = 8;
const byte redPWM = 0;
const byte greenPWM = 1;
const int freq = 500; // 5000 Khz produced a hum at 50% duty cycle, so brought this down
const byte knob_brightness = 100;


//hardware
void updateEncoder();
bool check_for_activity();
bool double_click( int btn_pin);
bool check_for_hold(int btn_pin, int hold_duration, void (*passed_fxn)());
bool check_for_hold(int btn_pin, int hold_duration, void (*passed_fxn)(os),os newos);
void blink_yellow(int num_blinks);
void pulse();


void updateEncoder(){
  int MSB = digitalRead(encoderPin1); //MSB = most significant bit
  int LSB = digitalRead(encoderPin2); //LSB = least significant bit
  int encoded = (MSB << 1) |LSB; //converting the 2 pin value to single number
  int sum  = (lastEncoded << 2) | encoded; //adding it to the previous encoded value
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) tempNow++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) tempNow--;
  lastEncoded = encoded; //store this value for next time
}

bool check_for_activity(){
  if (tempNow-tempOld>knob_sensitivity){
    return true;
  }
  if (tempNow-tempOld<-knob_sensitivity){
    return true;
  }
  if (!digitalRead(latch_BTN)){//
    return true;
  }
  if (!digitalRead(btn_1)){//
    return true;
  }
  if (!digitalRead(btn_2)){//
    return true;
  }
  if (!digitalRead(btn_3)){//
    return true;
  }
  if (!digitalRead(btn_4)){//
    return true;
  }
  return false;
}

bool double_click(int btn_pin){
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

bool check_for_hold(int btn_pin, int hold_duration, void (*passed_fxn)()){
    int hold_count = 0;
    bool hold_threshold_met = false;
    while(!digitalRead(btn_pin)){
      delay(1);
      hold_count++;
      if (hold_count>hold_duration){
        passed_fxn();
        while(!digitalRead(btn_pin)){
          delay(1);
        }
        hold_threshold_met = true;
      }
    }
    return hold_threshold_met;
}

bool check_for_hold(int btn_pin, int hold_duration, void (*passed_fxn)(os),os newos){
    int hold_count = 0;
    bool hold_threshold_met = false;
    while(!digitalRead(btn_pin)){
      delay(1);
      hold_count++;
      if (hold_count>hold_duration){
        passed_fxn(newos);
        while(!digitalRead(btn_pin)){
          delay(1);
        }
        hold_threshold_met = true;
      }
    }
    return hold_threshold_met;
}

void blink_yellow(int num_blinks){
  for (int i=0; i<num_blinks;i++){
    ledcWrite(greenPWM,0);
    delay(250);
    ledcWrite(greenPWM,knob_brightness);
    delay(250);
  }
}

void pulse(){
  for (int i = 255; i >knob_brightness; i--){
    ledcWrite(greenPWM,i);
    delay(1);
  }
}