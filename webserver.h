//Library need to create webserver
#include <WiFi.h>
#include <WebServer.h>

//Create variables to save event [Day/Time/Event_Name]
//String event_1[3] = {"23/6","+","+"};
//String event_2[3] = {"+","+","+"};
//String event_3[3] = {"+","+","+"};
//String event_4[3] = {"+","+","+"};
String event[4][3] = {{"+","+","+"},{"+","+","+"},{"+","+","+"},{"+","+","+"}};

//Create an object server
WebServer server(80);

// HTML for the webserver 
/*
 * This html code create a table, which has 4row&4colum
 * the row 1 and 3 spend for the name
 * the row 2 and 4 for the place that you put your time
 * the colum 2 and 4 for the place that  display the time that you have writen
 * when Submit button is pressed it mean you change the demain of the webserver to "/set_1", "/set_2", ...
 */
const char MAIN_page[]PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>YOUR CALENDAR</title>
</head>
<body>
  <h2>Set your calendar</h2>
  <table width="600" height="400" border="1" align="left" cellpadding="2" cellspacing="0">
    <tr><th>Event 1</th><th>Your Event 1</th>
      <th>Event 2</th><th>Your Event 2</th>
    </tr>
    <tr>
      <td><form action= "/set_1" >
        Date: <br>
        (Form: dd/mm, ex: 24/11) <br>
        <input type="text" name="Hours_1">
        <br>
        Minutes:<br>
        (Form: h:m, ex: 24:15) <br>
        <input type="text" name="Minutes_1">
        <br>
        Event: <br>
        <input type="text" name="Event_1">
        <br><br>
        <input type="submit" name="Submit_1">
      </form>
      </td>
      <td>
        <div id= 'event1'></div>
      </td>
      <td><form action= "/set_2" >
        Date:<br>
        (Form: dd/mm, ex: 24/11) <br>
        <input type="text" name="Hours_2">
        <br>
        Minutes:<br>
        (Form: h:m, ex: 20:15) <br>
        <input type="text" name="Minutes_2">
        <br>
        Event: <br>
        <input type="text" name="Event_2">
        <br><br>
        <input type="submit" name="Submit_2">
      </form>
      </td>
      <td>
        <div id= 'event2'></div>
      </td>
    </tr>
    <tr><th>Event 3</th><th>Your Event 3</th>
      <th>Event 4</th><th>Your Event 4</th>
    </tr>
    <tr>
      <td><form action= "/set_3" >
        Date:<br>
        (Form: dd/mm, ex: 24/11) <br>
        <input type="text" name="Hours_3">
        <br>
        Minutes:<br>
        (Form: h:m, ex: 20:15) <br>
        <input type="text" name="Minutes_3">
        <br>
        Event: <br>
        <input type="text" name="Event_3">
        <br><br>
        <input type="submit" name="Submit_3">
      </form>
      </td>
      <td>
        <div id= 'event3'></div>
      </td>
      <td><form action= "/set_4" >
        Date:<br>
        (Form: dd/mm, ex: 24/11) <br>
        <input type="text" name="Hours_4">
        <br>
        Minutes:<br>
        (Form: h:m, ex: 20:15) <br>
        <input type="text" name="Minutes_4">
        <br>
        Event: <br>
        <input type="text" name="Event_4">
        <br><br>
        <input type="submit" name="Submit_4">
      </form>
      </td>
      <td>
        <div id= 'event4'></div>
      </td>
    </tr>
  </table>
</body>
<script>
  window.onload= apdate;
  function apdate() {
    var xhttp1 = new XMLHttpRequest();
    var xhttp2 = new XMLHttpRequest();
    var xhttp3 = new XMLHttpRequest();
    var xhttp4 = new XMLHttpRequest();
    xhttp1.onreadystatechange = function(){
      if(this.readyState == 4 && this.status == 200){
        document.getElementById('event1').innerHTML = this.responseText
        apdate();
      }
    }
    xhttp2.onreadystatechange = function(){
      if(this.readyState == 4 && this.status == 200){
        document.getElementById('event2').innerHTML = this.responseText
        apdate();
      }
    }
    xhttp3.onreadystatechange = function(){
      if(this.readyState == 4 && this.status == 200){
        document.getElementById('event3').innerHTML = this.responseText
        apdate();
      }
    }
    xhttp4.onreadystatechange = function(){
      if(this.readyState == 4 && this.status == 200){
        document.getElementById('event4').innerHTML = this.responseText
        apdate();
      }
    }
    xhttp1.open('GET', '/event1', true);
    xhttp1.send();
    xhttp2.open('GET', '/event2', true);
    xhttp2.send();
    xhttp3.open('GET', '/event3', true);
    xhttp3.send();
    xhttp4.open('GET', '/event4', true);
    xhttp4.send();
  }
</script>
</html>)=====";

void handleRoot(){
  String s = MAIN_page;
  server.send(200, "text/html", s);
}

void handleForm_1(){
  //server.arg("String") get the value from name
  event[0][0] = server.arg("Hours_1");
  event[0][1] = server.arg("Minutes_1");
  event[0][2] = server.arg("Event_1");
  Serial.println("Event_1");
  Serial.println("Date: "+ event[0][0]);
  Serial.println("Hours: "+ event[0][1]);
  Serial.println("Even: "+ event[0][2]);
  //String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", MAIN_page);
}
void handleForm_2(){
  event[1][0] = server.arg("Hours_2");
  event[1][1] = server.arg("Minutes_2");
  event[1][2] = server.arg("Event_2");
  Serial.println("Event_2");
  Serial.println("Date: "+ event[1][0]);
  Serial.println("Hours: "+ event[1][1]);
  Serial.println("Even: "+ event[1][2]);

  //String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", MAIN_page);
}

void handleForm_3(){
  event[2][0] = server.arg("Hours_3");
  event[2][1] = server.arg("Minutes_3");
  event[2][2] = server.arg("Event_3");
  Serial.println("Event_3");
  Serial.println("Date: "+ event[2][0]);
  Serial.println("Hours: "+ event[2][1]);
  Serial.println("Even: "+ event[2][2]);
  //String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", MAIN_page);
}

void handleForm_4(){
  event[3][0] = server.arg("Hours_4");
  event[3][1] = server.arg("Minutes_4");
  event[3][2] = server.arg("Event_4");
  Serial.println("Event_4");
  Serial.println("Date: "+ event[3][0]);
  Serial.println("Hours: "+ event[3][1]);
  Serial.println("Even: "+ event[3][2]);
  //String s = "<a href='/'> Go Back </a>";
  server.send(200, "text/html", MAIN_page);
}

void start_Webserver(String wifiname, String wifipass){
  //WiFi.begin(wifiname, wifipass);
//  while(WiFi.status()!= WL_CONNECTED){
//    delay(500);
//    Serial.print(".");
//  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println("WiFi");
  Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
  IPAddress broadCast = WiFi.localIP();
  //broadCast[3] = 255;
  Serial.println(broadCast);
  background.setTextColor(TFT_WHITE);
  background.drawString(WiFi.localIP().toString(), 120, 92, 2);
  background.pushSprite(0,55);
  //default demain
  server.on("/", handleRoot);
  
  //When Submit button is pressed it call this function to save the values on the webserver to variabels on esp32
  //because webserver can't save the data that you put in, so we need to send it to esp32 and save them there
  server.on("/set_1", handleForm_1);
  server.on("/set_2", handleForm_2);
  server.on("/set_3", handleForm_3);
  server.on("/set_4", handleForm_4);
  
  //create a demain webpage
  //In each of these demain webpage has the String value 
  //Webserver go to these demain and get the value of them (by Get method) and display on the default webpage
  server.on("/event1", []{
    server.send(200, "text/html", event[0][2] + "<br>Date: " + event[0][0] + "<br>Time: " + event[0][1]);
      });
  server.on("/event2", []{
      server.send(200, "text/html", event[1][2] + "<br>Date: " + event[1][0] + "<br>Time: " + event[1][1]);
      });
  server.on("/event3", []{
      server.send(200, "text/html", event[2][2] + "<br>Date: " + event[2][0] + "<br>Time: " + event[2][1]);
      });
  server.on("/event4", []{
      server.send(200, "text/html", event[3][2] + "<br>Date: " + event[3][0] + "<br>Time: " + event[3][1]);
      });

  //strart Webserver
  server.begin();
}

void get_time_calen(){
  for(int i = 0; i < 4; i++){
    int pos_day = event[i][0].indexOf("/");
    int pos_time = event[i][1].indexOf(":");
    Serial.println("pos_time: " + String(pos_time));
    Serial.println("pos_day: " + String(pos_day));
    if(pos_time == -1){
      calen_time[i][0] = -1;
      calen_time[i][1] = -1;
      calen_time[i][2] = -1;
      calen_time[i][3] = -1;
    }
    else{
      calen_time[i][0] = event[i][0].substring(0, pos_day).toInt();
      calen_time[i][1] = event[i][0].substring(pos_day + 1, event[i][0].length()).toInt();
      calen_time[i][2] = event[i][1].substring(0, pos_time).toInt();
      calen_time[i][3] = event[i][1].substring(pos_time + 1, event[i][1].length()).toInt();
    }
    Serial.println("Event " + String(i + 1) + ":" + String(calen_time[i][0]) + "/" + String(calen_time[i][1]) + " " + String(calen_time[i][2])+":"+String(calen_time[i][3]));
    
  }
}
