#include "config_wifi.h"

//Varialbe for millis
unsigned long pre_time = 0;
//unsigned long servo_time = 0; 
int delay_time = 7000;


int16_t pos[4] = {-30, 75, 180, 285};//position in X axist of images in menu
uint8_t menu_options = 1;//for select tabs in menu 
uint8_t detail_option = 0;// for select categories in each tab in menu
int16_t eye[3][4] = {{70,53,16,78},{52,60,53,40},{53,45,33,57}};

//Variable for random value
int8_t x_ratio = 0, y_ratio = 0, change_rand = 0;
//Varialble for changing precess
uint8_t change_step1 = 0, change_step2 = 0;

#include "raw_func.h"

//Code setup for NTP client
#include "func_ntp.h" //function to get Time

//Create variable to save the infor of Wifi connected (Name, Pass)
String wifi_name;
String wifi_pass;

//
#include "OneButton.h"
#define dataA 32 //CLK pin den
#define dataB 35 //DT pin  nau
#define sw 34//tim
OneButton button(sw, true);
int8_t cur_value, pre_value;

//
#include "webserver.h"
int8_t start_webserver_state = 0;//to check the server start yet

///////////////////////////////////////
void setup() {
  Serial.begin(115200);
  Serial.println();
  //setup for servo
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servo1.setPeriodHertz(50);
  servo1.setPeriodHertz(50);
  servo1.attach(servo1_pin);//neck
  servo2.attach(servo2_pin);//belly
  servo1.write(90);
  servo2.write(90);
  servo1.detach();
  servo2.detach();

  pinMode(buz, OUTPUT);//setup for buzzer
  digitalWrite(buz, LOW);
  
  //setup for millis
  pinMode(dataA, INPUT);
  pinMode(dataB, INPUT);
  //pinMode(sw, INPUT_PULLUP);
  pre_value = digitalRead(sw);
  
  //clean FS, for testing
  //SPIFFS.format();

  //setup for tft display
  tft.init();
  tft.setRotation(0);
  tft.setSwapBytes(true);
  tft.fillScreen(TFT_BLACK);

  background.createSprite(239,131);
  img.createSprite(90,131);
  
  background.setSwapBytes(true);
  img.setSwapBytes(true);
  
  //read configuration from FS json
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);

 #if defined(ARDUINOJSON_VERSION_MAJOR) && ARDUINOJSON_VERSION_MAJOR >= 6
        DynamicJsonDocument json(1024);
        auto deserializeError = deserializeJson(json, buf.get());
        serializeJson(json, Serial);
        if ( ! deserializeError ) {
#else
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
#endif
        ////
        Serial.println("\nparsed json");
        strcpy(mqtt_server, json["mqtt_server"]);
        calen_time[0][0] = json["event_1"][0].as<uint8_t>();
        calen_time[0][1] = json["event_1"][1].as<uint8_t>();
        calen_time[0][2] = json["event_1"][2].as<uint8_t>();
        calen_time[0][3] = json["event_1"][3].as<uint8_t>();
        event[0][2] = json["event_1"][4].as<String>();

        calen_time[1][0] = json["event_2"][0].as<uint8_t>();
        calen_time[1][1] = json["event_2"][1].as<uint8_t>();
        calen_time[1][2] = json["event_2"][2].as<uint8_t>();
        calen_time[1][3] = json["event_2"][3].as<uint8_t>();
        event[1][2] = json["event_2"][4].as<String>();

        calen_time[2][0] = json["event_3"][0].as<uint8_t>();
        calen_time[2][1] = json["event_3"][1].as<uint8_t>();
        calen_time[2][2] = json["event_3"][2].as<uint8_t>();
        calen_time[2][3] = json["event_3"][3].as<uint8_t>();
        event[2][2] = json["event_3"][4].as<String>();

        calen_time[3][0] = json["event_4"][0].as<uint8_t>();
        calen_time[3][1] = json["event_4"][1].as<uint8_t>();
        calen_time[3][2] = json["event_4"][2].as<uint8_t>();
        calen_time[3][3] = json["event_4"][3].as<uint8_t>();
        event[3][2] = json["event_4"][4].as<String>();

        home_ser1 = json["servo"][0].as<uint8_t>();
        home_ser2 = json["servo"][1].as<uint8_t>();
        
        for(int i = 0; i <4; i++){
          for(int j = 0; j < 4; j++){
          Serial.println("Event_"+String(i)+": " + String(calen_time[i][j]));
          }
          Serial.println("Event_"+String(i)+": "+String(event[i][2]));
        }
      } 
      else {
        Serial.println("failed to load json config");
      }
      configFile.close();
      }
    }
  } else {
    Serial.println("failed to mount FS");
  }
  //end read

  // The extra parameters to be configured (can be either global or just in the setup)
  // After connecting, parameter.getValue() will get you the configured value
  // id/name placeholder/prompt default length
  WiFiManagerParameter custom_mqtt_server("server", "GMT", mqtt_server, 40);


  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  
  //add all your parameters here
  //wifiManager.addParameter(&custom_gmt_config);
  wifiManager.addParameter(&custom_mqtt_server);


  //reset settings - for testing
  //wifiManager.resetSettings();

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AutoConnectAP", "0907331893")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  wifi_name = wifiManager.getWiFiSSID();
  wifi_pass = wifiManager.getWiFiPass();

  //read updated parameters
  //(gmt_config, custom_gmt_config.getValue());
  strcpy(mqtt_server, custom_mqtt_server.getValue());
  
  Serial.println("The values in the file are: ");
  //Serial.println("\tapi_gmt : " + String(gmt_config));
  Serial.println("\tmqtt_server : " + String(mqtt_server));
  
  //When wifi is connected, start display the background Time
  tft.setFreeFont(&DSEG7_Classic_Bold_66);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString(":",116,82);
  tft.setTextColor(TFT_GRAY);
  
  tft.drawNumber(8,15,82);//bcd 1
  tft.drawNumber(8,61,82);//bcd 2
  tft.drawNumber(8,128,82);//bcd 3
  tft.drawNumber(8,174,82);//bcd 4

  //get the value GMT from sever
  gmtOffset_sec =  String(mqtt_server).toInt();

  //caculate GMT offset and get Time
  daylightOffset_sec = 3599*gmtOffset_sec;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //
  save_data(calen_time, event, home_ser1, home_ser2);
}
int8_t toogle[4] = {0,0,0,0};

void loop() {
  printLocalTime(); //update Time
  
  button.tick();
  //////////////////////////
  if(cur_hour != hour or cur_minute != minute){
    for(int i = 0; i < 4; i++){  //day/month/hour/minute
      if((hour == calen_time[i][2]) && (minute == calen_time[i][3])){
        if(toogle[i] == 0){ //to check event "i" happened yet and activate the alarm action only one time
          if((calen_time[i][0] == 0) && (calen_time[i][1] == 0)){
            Serial.println("TImeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
            main_state_display += 20;
            toogle[i] = 1;
            Serial.println("Main state: " + String(main_state_display));
            Serial.println("Toogle:" + String(toogle[i]));
            detail_option = 0;
            break;
          }
          else if((calen_time[i][0] == day) && (calen_time[i][1] == month)){
            Serial.println("Wakeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee");
            main_state_display += 20;
            toogle[i] = 1;
            Serial.println("Main state: " + String(main_state_display));
            Serial.println("Toogle:" + String(toogle[i]));
            detail_option = 0;
            break;
          }
        }
      }
        else {
        toogle[i] = 0;
        //Serial.println("Toogle  = 0");
        }
    }
  }
  if(main_state_display > 20){
    Serial.println("ALarm");
    options();
    if(detail_option != 0){
      main_state_display -= 20;
      Serial.println("Main state: " + String(main_state_display));      
      detail_option = 0;
      digitalWrite(buz, LOW);
      for(int i = 0; i<4;i++){toogle[i] = 1;}
      Serial.println("Close alarm");
    }
    if(detail_option == 0){
      for(int i = 0;i < 4; i ++){
        if(toogle[i] == 1){
          draw_alarm(calen_time[i][0],calen_time[i][1],calen_time[i][2],calen_time[i][3],event[i][2]);
        }
      }
      buzzer_warning();
    }
  }
  //////////////////////////
  
  //main_value_display = 3 for the Menu display
  else if(main_state_display < 3){
    //Serial.println("Main");
    main_rotary_encoder(); // read the motivation of encoder and return the value of main_state_display
    if(main_state_display == 1){
      eye_close(change_rand, 1, x_ratio, y_ratio, &delay_time);
      cur_hour = 25; //restart for display_time function
    }
    else if(main_state_display == 2){
      display_time();
    }
  }
  else if (main_state_display == 6){
    calendar();
    if(start_webserver_state == 1) server.handleClient();
  }
  else if (main_state_display == 7){
    if(set_angle_state == 0){
      setting_angle();      
    }
    else if(set_angle_state == 1){
      options_ser(&home_ser1, true);
      button.attachClick(change_state);
    }
    else if(set_angle_state == 2){
      options_ser(&home_ser2, false);
      button.attachClick(change_state);
    }
  }
  else if (main_state_display == 8){
    resest_menu();
  }
  else{//when main_state_display = 3 display Menu
    //Serial.println("Menu");
    menu_rotary_encoder();
  }
  //Serial.println(event_1[0]);
  
}

void start_display_menu(){
      main_state_display = 3;
      //pressed = 1; un_press = 1;
      tft.fillScreen(TFT_BLACK);
      background.fillSprite(TFT_BLACK);    
      img.pushImage(0, 0, 90, 130, round_re21);
      img.pushToSprite(&background, 75, 0);
      img.pushImage(0, 0, 90, 130, round_re22);
      img.pushToSprite(&background, 180, 0);
      img.pushImage(0, 0, 90, 130, round_re11);
      img.pushToSprite(&background, -30, 0);
      background.pushSprite(0, 55);
      //pre_time = millis();
      Serial.println("Start Menu");
}
void main_rotary_encoder(){
  cur_value = digitalRead(dataA);
  //cur_sw = digitalRead(sw);
  button.attachClick(start_display_menu);
  
  if(pre_value == 0 and cur_value == 1){
    if(digitalRead(dataB) == 1){
      main_state_display++;
      //delay_time = 10;
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_WHITE);
      tft.drawString(":",116,82);
    } 
    else{
      main_state_display--;
      //delay_time = 10;
      tft.fillScreen(TFT_BLACK);   
      tft.setTextColor(TFT_WHITE);
      tft.drawString(":",116,82);   
    }
    if(main_state_display > 2){
      main_state_display = 1;
    }
    else if(main_state_display < 1){
      main_state_display = 2;
    }
    ////////////////
    servo1.attach(servo1_pin);
    servo2.attach(servo2_pin);
    servo_check(&ang_ser1, home_ser1, 1);
    servo1.detach();
    servo1.detach();
    change_step1 = 0; change_step2 = 0;
    change_rand = random(-10,10);
    x_ratio = random(-8,8);
    y_ratio = random(-6,6);
    delay_time = 10;
    ////////////////
  }
    pre_value = cur_value;
    //Serial.println(main_state_display);
}
void display_time(){
  if(hour != cur_hour or minute != cur_minute){  
    //tft.fillScreen(TFT_BLACK);
    //tft.setTextColor(TFT_WHITE);
    //tft.drawString(":",116,82);
    if(hour > 9){ //display 2 value bcd
      tft.setTextColor(TFT_GRAY);
      tft.drawNumber(8,15,82);//bcd 1
      tft.drawNumber(8,61,82);//bcd 2
      tft.setTextColor(TFT_WHITE);
      tft.drawNumber(int(hour/10),15,82);//bcd hour
      tft.drawNumber(int(hour%10),61,82);//bcd hour
    }
    else{
      tft.setTextColor(TFT_GRAY);
      tft.drawNumber(8,15,82);//bcd 1
      tft.drawNumber(8,61,82);//bcd 2
      tft.setTextColor(TFT_WHITE);
      tft.drawNumber(hour,61,82);//bcd hour
    }

    if(minute > 9){
      Serial.println("minute1");
      tft.setTextColor(TFT_GRAY);
      tft.drawNumber(8,128,82);//bcd 1
      tft.drawNumber(8,174,82);//bcd 2
      tft.setTextColor(TFT_WHITE);
      tft.drawNumber(minute/10,128,82);//bcd minute
      tft.drawNumber(minute%10,174,82);//bcd minute
    }
    else{
      Serial.println("minute2");
      tft.setTextColor(TFT_GRAY);
      tft.drawNumber(8,128,82);//bcd 1
      tft.drawNumber(8,174,82);//bcd 2 
      tft.setTextColor(TFT_WHITE);
      tft.drawNumber(0,128,82);//bcd minute
      tft.drawNumber(minute,174,82);//bcd minute
    }
    cur_hour = hour;
    cur_minute = minute;
    //tft.invertDisplay(false);
  }
}
void option_move_menu(){
  for(int i = 0; i < 50; i+= 10){
    for(int j = 0; j < 4; j++){
      if(pos[j] > 100){
        pos[j] += 10;
      }
      if(pos[j] < 100){
        pos[j] -= 10;
      }
    }
    background.fillSprite(TFT_BLACK);
    background.fillRoundRect(105, 15, i*2.5, 100, 10, TFT_GRAY2);
    //img.pushToSprite(&background, 115, 0);
    //background.pushSprite(0,55);
    //background.fillSprite(TFT_BLACK);
    
    img.pushImage(0, 0, 90, 130, round_re11);
    img.pushToSprite(&background, pos[0], 0);
    img.pushImage(0, 0, 90, 130, round_re21);
    img.pushToSprite(&background, pos[1], 0);
    img.pushImage(0, 0, 90, 130, round_re22);
    img.pushToSprite(&background, pos[2], 0);
    img.pushImage(0, 0, 90, 130, round_re23);
    img.pushToSprite(&background, pos[3], 0);
    
    background.pushSprite(0,55);
       
  }
}
void option_back_menu(){
  for(int i = 0; i < 50; i+= 10){
    for(int j = 0; j < 4; j++){
      if(pos[j] > 100){
        pos[j] -= 10;
      }
      if(pos[j] < 100){
        pos[j] += 10;
      }
    }
    background.fillSprite(TFT_BLACK); 
    img.fillSprite(TFT_BLACK);
    img.fillRect(0, 5, 90 - i*1.8, 120, TFT_GRAY);
    img.pushToSprite(&background, 115, 0);
    
    img.pushImage(0, 0, 90, 130, round_re11);
    img.pushToSprite(&background, pos[0], 0);
    img.pushImage(0, 0, 90, 130, round_re21);
    img.pushToSprite(&background, pos[1], 0);
    img.pushImage(0, 0, 90, 130, round_re22);
    img.pushToSprite(&background, pos[2], 0);
    img.pushImage(0, 0, 90, 130, round_re23);
    img.pushToSprite(&background, pos[3], 0);
    
    background.pushSprite(0,55);
    //background.fillSprite(TFT_BLACK);
  }
}
void select_menu(){
  Serial.println("pressedddd");
  if(menu_options != 4){
    option_move_menu();
    background.setTextColor(TFT_WHITE);
    if(menu_options == 1){
      //Set calendar
      main_state_display = 6;
      background.drawString("/Calendar/",127,22,2);
      background.setTextColor(TFT_GRAY3);
      background.drawString("Start",127,40,2);
      background.drawString("Return",127,66,2);
    }
    else if(menu_options == 2){
      //Setting angle
      main_state_display = 7;
      background.drawString("/Setting/",127,22,2);
      background.setTextColor(TFT_GRAY3);
      background.drawString("Servo1",127,40,2);
      background.drawString("Servo2",127,66,2);
      background.drawString("Return",127,92,2);
    }
    else if(menu_options == 3){
      //resest
      main_state_display = 8;
      background.drawString("/Resest/",127,22,2);
      background.setTextColor(TFT_GRAY3);
      background.drawString("Start",127,40,2);
      background.drawString("Return",127,66,2);
    }
    background.pushSprite(0,55);
  }
  else{
    //return
    main_state_display = 1;
    menu_options = 1;
    pos[0] = -30;
    pos[1] = 75;
    pos[2] = 180;
    pos[3] = 285;
    Serial.println("End Menu");
  }
        //un_press = pressed;
        //pre_time2 = millis();
  Serial.println("Pressed");
        
}
void menu_rotary_encoder(){
  cur_value = digitalRead(dataA);
  //pressed = digitalRead(sw);
  button.attachClick(select_menu);
  
  if((pre_value == 0) && (cur_value == 1)){
    if(digitalRead(dataB) == 1){
      //decrease
      menu_options--;
      if(menu_options < 1) menu_options = 4;

      Serial.println("options : " + String(menu_options));
      //Serial.println("decrease");
      for(int z = 0; z < 7; z++){
        for(int i = 0; i < 4; i++){
          pos[i] -= 15;
          if(pos[i] < -135) pos[i] = 285 - 15;
          Serial.print(String(pos[i]) + " ");
        }
//        Serial.println(pos);
        img.pushImage(0, 0, 90, 130, round_re11);
        img.pushToSprite(&background, pos[0], 0);
        img.pushImage(0, 0, 90, 130, round_re21);
        img.pushToSprite(&background, pos[1], 0);
        img.pushImage(0, 0, 90, 130, round_re22);
        img.pushToSprite(&background, pos[2], 0);
        img.pushImage(0, 0, 90, 130, round_re23);
        img.pushToSprite(&background, pos[3], 0);
        background.pushSprite(0,55);
        background.fillSprite(TFT_BLACK);
        
        Serial.println("");
      }
    } 
    else{
      //increase  
      menu_options++;
      if(menu_options > 4) menu_options = 1;
      Serial.println("options : " + String(menu_options));
      Serial.println("increase");
      
      for(int z = 0; z < 7; z++){ 
        for(int i = 0; i < 4; i++){
          pos[i] += 15;
          if(pos[i] > 285) pos[i] = -135 + 15;          
          Serial.print(String(pos[i]) + " ");          
        }
        img.pushImage(0, 0, 90, 130, round_re11);
        img.pushToSprite(&background, pos[0], 0);
        img.pushImage(0, 0, 90, 130, round_re21);
        img.pushToSprite(&background, pos[1], 0);
        img.pushImage(0, 0, 90, 130, round_re22);
        img.pushToSprite(&background, pos[2], 0);
        img.pushImage(0, 0, 90, 130, round_re23);
        img.pushToSprite(&background, pos[3], 0);
        background.pushSprite(0,55);
        background.fillSprite(TFT_BLACK);
        
        Serial.println("");
      }
    }
  }
  
  pre_value = cur_value;
}
void options(){
  cur_value = digitalRead(dataA);
  //button.attachClick();

  if((pre_value == 0) && (cur_value == 1)){
    if(digitalRead(dataB) == 1){
      detail_option --;
    }
    else{
      detail_option ++;
    }
  }
  pre_value = cur_value;
}
void choose_setting_angle(){
  if(detail_option == 1){
    set_angle_state = 1;
    servo1.attach(servo1_pin);
    servo2.attach(servo2_pin);
  }
  else if(detail_option == 2){
    set_angle_state = 2;
    servo1.attach(servo1_pin);
    servo2.attach(servo2_pin);
  }
  else if(detail_option == 3){
    option_back_menu();
    main_state_display = 3;
    detail_option = 0;
    set_angle_state = 0;
    save_data(calen_time, event, home_ser1, home_ser2);
    servo1.detach();
    servo2.detach();
  }
}
void setting_angle(){
  options();
  button.attachClick(choose_setting_angle);
  if(detail_option < 1) detail_option = 3;
  if(detail_option > 3) detail_option = 1;

  if(detail_option == 1){
    background.setTextColor(TFT_WHITE);
    background.drawString("Servo1",127,40,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Servo2",127,66,2);
    background.drawString("Return",127,92,2);
  }
  else if(detail_option == 2){
    background.setTextColor(TFT_WHITE);
    background.drawString("Servo2",127,66,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Servo1",127,40,2);
    background.drawString("Return",127,92,2);
  }
  else if(detail_option == 3){
    background.setTextColor(TFT_WHITE);
    background.drawString("Return",127,92,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Servo2",127,66,2);
    background.drawString("Servo1",127,40,2);
  }
  background.pushSprite(0,55);
}
void choose_calendar(){
  if(detail_option == 1){
    start_Webserver(wifi_name, wifi_pass);
    start_webserver_state = 1;
//    background.setTextColor(TFT_WHITE);
//    background.drawString(WiFi.localIP(), 127, 92, 2);
//    background.pushSprite(0,55);
  }
  else if(detail_option == 2){
    server.close();
    Serial.println("CLose server");
    shouldSaveConfig = true;
    start_webserver_state = 0;
    get_time_calen();
    option_back_menu();
    save_data(calen_time, event, home_ser1, home_ser2);
    main_state_display = 3;
    detail_option = 0;
  }
}
void calendar(){
  options();
  button.attachClick(choose_calendar);
  if(detail_option > 2) detail_option = 1;
  if(detail_option < 1) detail_option = 2;

  if(detail_option == 1){
    background.setTextColor(TFT_WHITE);
    background.drawString("Start",127,40,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Return",127,66,2);
  }
  else if(detail_option == 2){
    background.setTextColor(TFT_GRAY3);
    background.drawString("Start",127,40,2);
    background.setTextColor(TFT_WHITE);
    background.drawString("Return",127,66,2);
  }
  background.pushSprite(0,55);
}
void choose_resest(){
  if(detail_option == 1){
    WiFiManager wm;    

    //reset settings - for testing
    //wm.resetSettings();
  
    // set configportal timeout
    wm.setConfigPortalTimeout(120);

    if (!wm.startConfigPortal("OnDemandAP")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.restart();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
  }
  else if(detail_option == 2){
    Serial.println("Resest");
    tft.fillRectVGradient(70,90,100,60,TFT_PINK,TFT_BLUE);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("...",100,95,6);
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    SPIFFS.format();
    ESP.restart();
  }
  else if(detail_option == 3){
    option_back_menu();
    main_state_display = 3;
    detail_option = 0;
  }
}
void resest_menu(){
  options();
  button.attachClick(choose_resest);
  if(detail_option < 1) detail_option = 3;
  if(detail_option > 3) detail_option = 1;

  if(detail_option == 1){
    background.setTextColor(TFT_WHITE);
    background.drawString("Config",127,40,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Reset",127,66,2);
    background.drawString("Return",127,92,2);
  }
  else if(detail_option == 2){
    background.setTextColor(TFT_WHITE);
    background.drawString("Reset",127,66,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Config",127,40,2);
    background.drawString("Return",127,92,2);
  }
  else if(detail_option == 3){
    background.setTextColor(TFT_WHITE);
    background.drawString("Return",127,92,2);
    background.setTextColor(TFT_GRAY3);
    background.drawString("Config",127,40,2);
    background.drawString("Reset",127,66,2);
  }
  background.pushSprite(0,55);
}

void options_ser(uint8_t *count, bool ser){
  cur_value = digitalRead(dataA);
  //button.attachClick();
  uint8_t angle = *count;
  if((pre_value == 0) && (cur_value == 1)){
    if(digitalRead(dataB) == 1){
      angle -=2;
    }
    else{
      angle +=2;
    }
    if(angle < 60) angle = 60;
    if(angle > 120) angle = 120;
    *count = angle;
    if(ser == true) {
      servo1.write(*count);
      Serial.println("Ser_1: " + String(*count));
      }
    else {
      servo2.write(*count);
      Serial.println("Ser_2: " + String(*count));
      }
  }
  pre_value = cur_value;
}

void change_state(){
  set_angle_state = 0;
}
