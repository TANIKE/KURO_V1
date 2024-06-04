////////////////Code setup for Round_tft_display////////////
#include <TFT_eSPI.h>
#include "font.h"
#include "font2.h"
#include "img_menu.h"

TFT_eSPI tft = TFT_eSPI();
//Sprite for display eyes
uint8_t main_state_display = 2; // variable for the main display

//Sprite for Menu
TFT_eSprite background = TFT_eSprite(&tft);
TFT_eSprite img = TFT_eSprite(&tft);

//variables for change colours in alarming
unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
uint8_t loop_variable = 0;
unsigned int colour = red << 11;
//Servo
#include <ESP32Servo.h>
Servo servo1, servo2;
#define servo1_pin 5
#define servo2_pin 18
uint8_t ang_ser1 = 90, home_ser1 = 90;
uint8_t ang_ser2 = 90, home_ser2 = 90;//angle of servo
uint8_t set_angle_state = 0; // use to as the condition for change ang_ser in setting angle of menu

void draw_Ellipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry,int32_t w, int32_t h, uint16_t color)
{
  if (rx<2) return;
  if (ry<2) return;
  int32_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  for(int i= y0 -h; i < y0 - ry; i++){
    img.drawLine(x0 - w,i, x0 + w, i, color);
    img.drawLine(x0 - w,-i + 2*y0, x0 + w, -i + 2*y0, color);
  }

  for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
    img.drawLine(x0 - w, y0 - y, x0 - x, y0 - y, color);
    img.drawLine(x0 + x + 1, y0 - y, x0 + w, y0 - y, color);
    
    img.drawLine(x0 - w, y0 + y, x0 - x, y0 + y, color);
    img.drawLine(x0 + x + 1, y0 + y , x0 + w, y0 + y , color);

    if (s >= 0) {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
    img.drawLine(x0 - w, y0 - y, x0 - x, y0 - y, color);
    img.drawLine(x0 + x + 1, y0 - y, x0 + w, y0 - y, color);
    
    img.drawLine(x0 - w, y0 + y, x0 - x, y0 + y, color);
    img.drawLine(x0 + x + 1, y0 + y, x0 + w, y0 + y, color);

    if (s >= 0) {
      s += fy2 * (1 - x);
      x--;
    }
    s += rx2 * ((4 * y) + 6);
  }
}
void draw_eyes(int change, int rx, int ry){
  int change_xsign = 1;
  int change_ysign = 1;
  
  if(rx < 0 ) change_xsign = -1; // to determine plus(+) or minor(-) the ratio change
  if(ry < 0) change_ysign = -1;
    background.fillSprite(TFT_BLACK);
    img.fillSprite(TFT_BLACK);
    img.fillRect(2,23,85,80,TFT_WHITE);
    draw_Ellipse(-3-eye[0][2] + 90,eye[0][3],eye[0][0],eye[0][1] - change,eye[0][0]+5,eye[0][1]+5,TFT_BLACK);  
    draw_Ellipse(-3-eye[1][2] + 90,eye[1][3] - change,eye[1][0],eye[1][1],eye[1][0]+5,eye[1][1]+5+change,TFT_BLACK);  
    draw_Ellipse(-3-eye[2][2] + 90,eye[2][3] - change,eye[2][0],eye[2][1],eye[2][0]+5,eye[2][1]+5,TFT_BLACK);  
    img.pushToSprite(&background,120 + rx + change_xsign*change/3,0 + ry + change_ysign*change/2);
    //Left eye
    img.fillSprite(TFT_BLACK);
    img.fillRect(2,23,85,80,TFT_WHITE);
    draw_Ellipse(eye[0][2],eye[0][3],eye[0][0],eye[0][1] - change,eye[0][0]+5,eye[0][1]+5,TFT_BLACK);  
    draw_Ellipse(eye[1][2],eye[1][3] - change,eye[1][0],eye[1][1],eye[1][0]+5,eye[1][1]+5+change,TFT_BLACK);  
    draw_Ellipse(eye[2][2],eye[2][3] - change,eye[2][0],eye[2][1],eye[2][0]+5,eye[2][1]+5,TFT_BLACK);  
    img.pushToSprite(&background,30 + rx + change_xsign*change_rand/3,0 + ry + change_ysign*change_rand/2);
    background.pushSprite(0,55);
}
void servo_check(uint8_t *current_angle, uint8_t destination, byte servo){
  uint8_t cur = *current_angle;
  while(cur != destination){      
    if(cur > destination) cur --;
    else if(cur < destination) cur ++;
    *current_angle = cur;
    Serial.println("INnNNNNN loop");
    
    if(servo == 1){
        servo1.write(*current_angle);
        //*current_angle += (destination - *current_angle);
        Serial.println("Servo1: " + String(*current_angle));
        Serial.println("Global 1: " +String(ang_ser1));
    }
    else if(servo == 2){
        servo2.write(*current_angle);
        //*current_angle += (destination - *current_angle);
        Serial.println("Servo2: " + String(*current_angle));
        Serial.println("Global 2: " +String(ang_ser2));
    }
    Serial.println("Loopp");
  } 
}


void eye_close(int limit, int speed_close, int x,int y, int* time_delay){
  int8_t change_sign = 1;
  int8_t change_x;
  int8_t change_y;

  if(x < 0) change_x = map(x,-8,0,-45,-8);
  else if(x > 0) change_x = map(x,0,8,8,45);
  
  if(y < 0) change_y = map(y,-6,0,-30,-8);
  else if(y > 0) change_y = map(y,0,6,8,40);
  
  if(limit < 0) change_sign = -1;
  if(change_step1 < abs(limit)){
    if(change_step1 < 2){
    servo1.attach(servo1_pin);
    servo2.attach(servo2_pin);
    Serial.println("attach11111111");
    }
    draw_eyes(change_step1*change_sign, change_step1*x, change_step1*y);
    servo_check(&ang_ser1, home_ser1 + (change_step1 + speed_close)*(change_y/abs(limit)), 1);
    servo_check(&ang_ser2, home_ser2 + (change_step1 + speed_close)*(change_x/abs(limit)), 2);
    change_step1 += speed_close;
    Serial.println("Map yyyyyyyyyyyyyyyyyyyy = " + String(change_y));   
    Serial.println("Map xxxxxxxxxxxxxxxxxxxx = " + String(change_x));
    
    Serial.println("Step1");
    if(change_step1 == abs(limit)){
      servo1.detach();
      servo2.detach();
      Serial.println("Detach servoooooooooooooooo");
    }
  }
  else if(change_step1 >= abs(limit) and change_step2 == 0){
    if(millis() - pre_time >= 2500){
      pre_time = millis();
      change_step2 = 1;        
    }
    Serial.println("Delay");
  }
  else if(change_step2 != 0  and change_step2 < abs(limit)){
    Serial.println("Step2 start");
    if(change_step2 < 2){
    servo1.attach(servo1_pin);
    servo2.attach(servo2_pin);
    Serial.println("attach222222");
    }
    draw_eyes(limit - change_step2*change_sign,x*(change_step1 - change_step2),y*(change_step1 - change_step2));
    servo_check(&ang_ser1, home_ser1 + (change_step1-speed_close - change_step2)*(change_y/abs(limit)), 1);
    servo_check(&ang_ser2, home_ser2 + (change_step1-speed_close - change_step2)*(change_x/abs(limit)), 2);
    change_step2 += speed_close;  
    Serial.println("Step2");  
    if(change_step2 == abs(limit)){
      servo1.detach();
      servo2.detach(); 
      Serial.println("detach2"); 
    }
  }
  else if(change_step2 >= abs(limit)){
    if(millis() - pre_time >= *time_delay){
      change_step1 = 0; change_step2 = 0;
      change_rand = random(-10,10);
      x_ratio = random(-8,8);
      y_ratio = random(-6,6);
      Serial.println("Clear");
      pre_time = millis();
      *time_delay = 50000;
    }
  }
}

void draw_alarm(int day1, int month1,int hour1, int minute1, String text){
  if (targetTime < millis()) {
    targetTime = millis() + 100;//10000;
    // Colour changing state machine
    if(loop_variable < 160) {      
      background.fillScreen(TFT_BLACK);
      background.setTextColor(colour);
      //backfround.unloadFont();
      background.drawString(String(day1)+"/"+String(month1),10,17,2);
      background.drawString(text, 15, 100, 2);
      
      background.setFreeFont(&DSEG7_Classic_Bold_66);
      background.drawString(":",116,27);
      //background.setTextColor(TFT_GRAY);
      
      if(hour1 > 9){ //display 2 value bcd
      //tft.setTextColor(TFT_GRAY);
      background.drawNumber(int(hour1/10),15,27);//bcd hour
      background.drawNumber(int(hour1%10),61,27);//bcd hour
    }
    else{
      background.drawNumber(hour1,61,27);//bcd hour
    }

    if(minute1 > 9){
      Serial.println("minute1");
      background.drawNumber(minute1/10,128,27);//bcd minute
      background.drawNumber(minute1%10,174,27);//bcd minute
    }
    else{
      Serial.println("minute2");
      background.drawNumber(0,128,27);//bcd minute
      background.drawNumber(minute1,174,27);//bcd minute
    }
    background.pushSprite(0,55);
      switch (state) {
        case 0:
          green += 4;
          if (green == 64) {
            green = 63;
            state = 1;
          }
          break;
        case 1:
          red-=4;
          if (red == 255) {
            red = 0;
            state = 2;
          }
          break;
        case 2:
          blue +=4;
          if (blue == 32) {
            blue = 31;
            state = 3;
          }
          break;
        case 3:
          green -= 4;
          if (green == 255) {
            green = 0;
            state = 4;
          }
          break;
        case 4:
          red +=4;
          if (red == 32) {
            red = 31;
            state = 5;
          }
          break;
        case 5:
          blue -=4;
          if (blue == 255) {
            blue = 0;
            state = 0;
          }
          break;
      }
      colour = red << 11 | green << 5 | blue;
      loop_variable ++;
    }
    if(loop_variable >= 160) loop_variable = 0;
  }
}
