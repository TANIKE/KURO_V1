#include <WiFi.h>
#include "time.h"
const char* ntpServer = "in.pool.ntp.org";
uint8_t  gmtOffset_sec = 0;
uint16_t   daylightOffset_sec = 3599*gmtOffset_sec;//GMT
String weekDays[8]={"","Sun", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat"};
uint8_t cur_hour = 0,cur_minute = 0, hour, minute, day , month, weekday;
uint8_t calen_time[4][4] = {{10,12,14,16},{0,0,0,0},{0,0,0,0},{0,0,0,0}};//day/month/hour/minute

#define buz 25
unsigned long buz_delay = 0;
bool buz_state = true;
void buzzer_warning(){
  if(millis() - buz_delay >= 150){
    if(buz_state == true){
      digitalWrite(buz, HIGH);
      
      Serial.println("ONNNNNNNN");
      buz_state = false;
    }
    else{
      digitalWrite(buz, LOW);
      Serial.println("OFFFFFFFF");
      buz_state = true;
    }
    buz_delay = millis();
  }
}

void printLocalTime(){
  struct tm timeinfo;
  //int minute = tm_min;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  //Serial.println(&timeinfo);
  hour = *&timeinfo.tm_hour;
  minute = *&timeinfo.tm_min;
  day = *&timeinfo.tm_mday;
  month = *&timeinfo.tm_mon + 1;
  weekday = *&timeinfo.tm_wday;
  Serial.println("hour: " + String(hour));
  Serial.println("hour: " + String(minute));
}
