//bluetooth 
void connecting();
void print_version();
void notify(int fkey);
void shortcut(char ltr);
void change_view(view new_view = gallery);
void leave();
void shutdown();

void connecting(){
  last_active = millis();
  ledcWrite(greenPWM,0);
  bool inhaling = true;
  byte red_brightness = knob_brightness;
  while(!bleKeyboard.isConnected()){
    delay(2);
    if (!digitalRead(btn_5)){
      check_for_hold(btn_5, shutdown_hold, shutdown);
    }
    if (inhaling){
      if (red_brightness<255){
        red_brightness++;
        ledcWrite(redPWM,red_brightness);
      }
      else{
        inhaling = false;
      }
    }
    else{
      if (red_brightness>0){
        red_brightness--;
        ledcWrite(redPWM,red_brightness);
      }
      else{
        inhaling = true;
      }
    }
    if (millis() - last_active > connection_timer){
      shutdown();
    }
  }
  delay(500);
  ledcWrite(redPWM,0);
  ledcWrite(greenPWM,knob_brightness);
}

void print_version(){
  bleKeyboard.print("version:");
  bleKeyboard.println(VERSION);
  bleKeyboard.print("os:");
  bleKeyboard.println(active_os);
  bleKeyboard.print("mode:");
  bleKeyboard.println(active_mode);
}

void notify(int fkey){
  bleKeyboard.press(KEY_LEFT_CTRL);
  bleKeyboard.press(KEY_LEFT_GUI);
  bleKeyboard.press(KEY_LEFT_ALT);
  bleKeyboard.press(fkey);
  bleKeyboard.releaseAll();
}

void shortcut(char ltr){
  if (active_os == mac){
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

void change_view(view new_view){
  if (active_os == mac){
    shortcut('w');
  }
  else{
    bleKeyboard.press(KEY_LEFT_ALT);
    if (new_view == gallery){
      bleKeyboard.press(KEY_F2);
    }
    else if (new_view == speaker){
      bleKeyboard.press(KEY_F1);
    }
    bleKeyboard.releaseAll();
  }
}

void leave(){
  if (active_os == mac){
    bleKeyboard.press(KEY_LEFT_GUI);
    bleKeyboard.press('w');
  }
  else{
    bleKeyboard.press(KEY_LEFT_ALT);
    bleKeyboard.press('q');
  }

  bleKeyboard.releaseAll();
  delay(250);
  bleKeyboard.write(KEY_RETURN);
  last_chance(8000);
}

void shutdown(){
  EEPROM.write(MODE_BYTE,active_mode);
  EEPROM.write(OS_BYTE,active_os);
  EEPROM.commit();
  ledcWrite(greenPWM,0);
  ledcWrite(redPWM,0);
  pinMode(latch_BTN,OUTPUT);
  digitalWrite(latch_BTN,LOW);
}