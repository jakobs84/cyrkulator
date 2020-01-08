//nodeMCU v1.0 (black) with Arduino IDE
//stream temperature data DS18B20 with 1wire on ESP8266 ESP12-E (nodeMCU v1.0)
//Temperature measurement
//Timer switch with 4 timers
//switch temperature dependend as well
//publish temperature data to thingspeak
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
//Time stuff
#include <Time.h>
#include <TimeLib.h>
//file system
#include "FS.h"

//webserver
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include <DNSServer.h>
#include <WiFiManager.h>


ESP8266WebServer server(80);
MDNSResponder mdns;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 7200, 600000);





String webPage, webPage1, webPage2;
int count = 0;
int WFTMP = 1;
int gpio0_pin = 0;
int gpio2_pin = 12;
//time stuff

uint32_t currentMillis = 0;
uint32_t previousMillis = 0;




//webserver
//Def
#define myPeriodic 600 // in sec | Thingspeak pub is 10 min
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float prevTemp = 0;
float SwitchTemp1, SwitchTemp2;
const char* IOTserver = "api.thingspeak.com";
String apiKey = "your api";
//String MY_SSID = "your ssid";
//String MY_PWD = "you pwd";
String tempC;
String tempC1;
String tempV;
String tempW, CURR_time, CURR_day, TEMP1_SET, TEMP2_SET, TON_HR_set, TOFF_HR_set, TON_MIN_set, TOFF_MIN_set, T_DAY_set;
int sent = 0;
int row = 6;
char temperatureString[6];

float temp;
float temp1;


String tmax_state;
String tmin_state;
String TON_HR_ARR;
String TOFF_HR_ARR;
String TON_MIN_ARR;
String TOFF_MIN_ARR;
String T_DAY_ARR;




String TON[6], TOFF[6];
String TDAY[4][7];
//String TIMER_SET;
String TIMER_SET = "OFF" ;

void setup() {
  //webserver
  WiFi.mode(WIFI_STA);
  // preparing GPIOs and set to low only one is used so far
  pinMode(gpio0_pin, OUTPUT);
  pinMode(gpio2_pin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);

  webPage1 = "";
  webPage1 += "﻿<!DOCTYPE html>\n";
  webPage1 += "<html>\n";
  webPage1 += "<head>\n";
  webPage1 += "    <title> Circulation Pump </title>\n";
  webPage1 += "  <meta charset='utf-8' />\n";
  webPage1 += "    <script type='text/javascript'>\n";
  webPage1 += "    </script> \n";
  webPage1 += "</head>";
  webPage1 += "<body onload='BTN_COL(); Dayselect()'>";
  //webPage1 +="<body>";
  webPage1 += "    <h1>Cyrkulator </h1> ";
  webPage1 += "    <iframe src='http://free.timeanddate.com/clock/i5mu6weg/n37/tt0/tw0/tm1/ts1/tb4' frameborder='0' width='87' height='34'>";
  webPage1 += "    </iframe>";
  webPage1 += "    <p></p>";

  webPage2 = "";
  //webPage2 +="    <form method='post'>";
  /*  webPage2 += "    <form>";
    webPage2 += "        Temperature Maximum";
    webPage2 += "        <p></p>";
    webPage2 += "        <input id='rangeMAX' type='range' name='TempMAX' min='0' max='100' step='1' onchange='updateTextInputMAX(this.value)' ; />";
    webPage2 += "        <input type='text' id='textInputMAX' name='textInputMAX' >";
    webPage2 += "        <p></p>";
    webPage2 += "        Temperature Minimum";
    webPage2 += "        <p></p>";
    webPage2 += "        <input id='rangeMIN' type='range' name='TempMIN' min='0' max='100' step='1' onchange='updateTextInputMIN(this.value)' ; />";
    webPage2 += "        <input type='text' id='textInputMIN' name='textInputMIN' >";
    webPage2 += "        <p><input type='submit' name='submitTMP' value='Send'></p>";
    webPage2 += "    </form>";
  */
  webPage2 += "    <form>";
  webPage2 += "<p>Pump ON/OFF <button name='ONBTN' value='ON' id='ONBTN' style='background-color: #00FF00 onclick='setColor()' >ON</button>";
  webPage2 += "               <button name='OFFBTN' value='OFF' id='OFFBTN' style='background-color: #00FF00' onclick='setColor1()' >OFF</button>";
  webPage2 += "               <button name='AUTOBTN' value='AUTO' id='AUTOBTN' style='background-color: #00FF00' onclick='setColor2()' >AUTO</button></p>";
  webPage2 += "    </form>";
  webPage2 += "    <form>";
  webPage2 += "<p>Reload Page<button name='RELOAD' id='RELOAD' onClick='refreshPage()'>RELOAD</button></p>";
  webPage2 += "    </form>";
  webPage2 += "    <form>";
  webPage2 += "<p>RESET WIFI Connection<button name='RESET_WIFI' value='ON' id='RESET_WIFI' >RESET_WIFI</button></p>";
  webPage2 += "<p>Connect to 192.168.4.1 after reset</p>";
  webPage2 += "    </form>";



  webPage2 += "    <script type='text/javascript'>\n"                              ;
  webPage2 += "                 var tontxt_HR = document.getElementById('txt3').textContent;"                              ;
  webPage2 += "                 tontxt_HR = tontxt_HR.trim();"                                                             ;
  webPage2 += "                 var TON_HR = tontxt_HR.split(',');"                                                        ;
  webPage2 += "                 var tontxt_MIN = document.getElementById('txt4').textContent;"                             ;
  webPage2 += "                 tontxt_MIN = tontxt_MIN.trim();"                                                           ;
  webPage2 += "                 var TON_MIN = tontxt_MIN.split(',');"                                                      ;
  webPage2 += "                 var tofftxt_HR = document.getElementById('txt5').textContent;"                             ;
  webPage2 += "                 tofftxt_HR = tofftxt_HR.trim();"                                                           ;
  webPage2 += "                 var TOFF_HR = tofftxt_HR.split(',');"                                                      ;
  webPage2 += "                 var tofftxt_MIN = document.getElementById('txt6').textContent;"                            ;
  webPage2 += "                 tofftxt_MIN = tofftxt_MIN.trim();"                                                         ;
  webPage2 += "                 var TOFF_MIN = tofftxt_MIN.split(',');"                                                    ;
  webPage2 += "                 var ttxt_DAY = document.getElementById('txt7').textContent;"                               ;
  webPage2 += "                 ttxt_DAY = ttxt_DAY.trim();"                                                               ;
  webPage2 += "                 var T_DAY = ttxt_DAY.split(',');"                                                          ;
  webPage2 += "var tempmax = document.getElementById('txt1').textContent;\n";
  webPage2 += "var tempmin = document.getElementById('txt2').textContent;\n";
  //  webPage2 += "var man_set = document.getElementById('txt8').textContent;\n";
  webPage2 += "document.getElementById(\"rangeMAX\").setAttribute('value', tempmax);\n";
  webPage2 += "document.getElementById(\"textInputMAX\").setAttribute('value', tempmax);\n";
  webPage2 += "document.getElementById(\"rangeMIN\").setAttribute('value', tempmin);\n";
  webPage2 += "document.getElementById(\"textInputMIN\").setAttribute('value', tempmin);\n";

  webPage2 += "                 function setColor()  {"                                                                    ;
  webPage2 += "                     document.getElementById('ONBTN').setAttribute('style', 'background-color: #f44336');"  ;
  webPage2 += "                     document.getElementById('OFFBTN').setAttribute('style', 'background-color: #00FF00');" ;
  webPage2 += "                     document.getElementById('AUTOBTN').setAttribute('style', 'background-color: #00FF00');";
  webPage2 += "                 }"                                                                                         ;
  webPage2 += "                 function setColor1() {"                                                                    ;
  webPage2 += "                     document.getElementById('OFFBTN').setAttribute('style', 'background-color: #f44336');" ;
  webPage2 += "                     document.getElementById('ONBTN').setAttribute('style', 'background-color: #00FF00');"  ;
  webPage2 += "                     document.getElementById('AUTOBTN').setAttribute('style', 'background-color: #00FF00');";
  webPage2 += "                 }"                                                                                         ;
  webPage2 += "                 function setColor2() {"                                                                    ;
  webPage2 += "                     document.getElementById('AUTOBTN').setAttribute('style', 'background-color: #f44336');";
  webPage2 += "                     document.getElementById('OFFBTN').setAttribute('style', 'background-color: #00FF00');" ;
  webPage2 += "                     document.getElementById('ONBTN').setAttribute('style', 'background-color: #00FF00');"  ;
  webPage2 += "                 }"                                                                                         ;
  webPage2 += "                 function refreshPage(){"                                                                    ;
  webPage2 += "                      window.location.reload();";
  webPage2 += "                 }"                                                                                         ;



  webPage2 += "               function Dayselect() {";


  webPage2 += "                    var frm = document.createElement('FORM');"       ;
  webPage2 += "                    var x = document.createElement('TABLE');"        ;
  webPage2 += "                    frm.setAttribute('id','dateform');"              ;
  webPage2 += "                    var frmsb = document.createElement('INPUT');"    ;
  webPage2 += "                    frmsb.setAttribute('type', 'submit');"           ;
  webPage2 += "                    frmsb.setAttribute('name', 'submit');"           ;
  webPage2 += "                    frmsb.setAttribute('value', 'Send');"            ;
  webPage2 += "                    document.body.appendChild(frm);"                 ;
  webPage2 += "                    x.setAttribute('id', 'myTable');"                ;
  webPage2 += "                    frm.appendChild(x);"                             ;
  webPage2 += "                    frm.appendChild(frmsb);"                         ;
  webPage2 += "                     var y1 = document.createElement('TH');"          ;
  webPage2 += "                    y1.setAttribute('id', 'myTh');"                  ;
  webPage2 += "                    var y = document.createElement('TR');"           ;
  webPage2 += "                    y.setAttribute('id', 'myTr');"                   ;
  webPage2 += "                    document.getElementById('myTable').appendChild(y);"                           ;
  webPage2 += "                    var wdays = ['TON', 'TOF', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun'];" ;
  webPage2 += "                    for (i in wdays) {"                                                           ;
  webPage2 += "                        var z = document.createElement('TH');"                                    ;
  webPage2 += "                        var name = wdays[i];"                                                      ;
  webPage2 += "                        var x1 = document.createTextNode(name);"                                  ;
  webPage2 += "                        z.appendChild(x1);"                                                       ;
  webPage2 += "                        document.getElementById('myTr').appendChild(z);"                          ;
  webPage2 += "                    }"                                                                            ;
  webPage2 += "                    var n = -1;"                                                                  ;
  webPage2 += "                    var f = -1;"                                                                  ;
  // jakaobs84                      for (k = 0; k < 4; k++) {"
  webPage2 += "                    for (k = 0; k < 6; k++) {"                                                    ;
  webPage2 += "                        var y2 = document.createElement('TR');"                                   ;
  webPage2 += "                        var mytr = 'myTr' + k;"                                                   ;
  webPage2 += "                        y2.setAttribute('id', mytr);"                                             ;
  webPage2 += "                        document.getElementById('myTable').appendChild(y2);"                      ;
  webPage2 += "                        for (i = 0; i < 9; i++) {"                                                ;
  webPage2 += "                            var z = document.createElement('TD');"                                ;
  webPage2 += "                            var x1 = document.createElement('INPUT');"                            ;
  webPage2 += "                            var x2 = document.createElement('INPUT');"                           ;
  webPage2 += "                            if (i <= 1) {"                                                        ;
  webPage2 += "                                var tmset1 = 'TH' + k + i;"                                       ;
  webPage2 += "                                var tmset2 = 'TM' + k + i;"                                       ;
  webPage2 += "                                x1.setAttribute('type', 'number');"                               ;
  webPage2 += "                                x1.setAttribute('id', tmset1);"                                 ;
  webPage2 += "                                x1.setAttribute('name', tmset1);"                                 ;
  webPage2 += "                                x1.setAttribute('min', '0');"                                     ;
  webPage2 += "                                x1.setAttribute('max', '23');"                                    ;
  webPage2 += "                                x2.setAttribute('type', 'number');"                               ;
  webPage2 += "                                x2.setAttribute('id', tmset2);"                                 ;
  webPage2 += "                                x2.setAttribute('name', tmset2);"                                 ;
  webPage2 += "                                x2.setAttribute('min', '0');"                                     ;
  webPage2 += "                                x2.setAttribute('max', '59');"                                    ;
  webPage2 += "                                if (i == 0) {"                                                    ;
  webPage2 += "                                      n = n + 1;"                                                 ;
  webPage2 += "                                      x1.setAttribute('value', TON_HR[k]);"                       ;
  webPage2 += "                                      x2.setAttribute('value', TON_MIN[k]);"                      ;
  webPage2 += "                                }"                                                                ;
  webPage2 += "                                if (i == 1) {"                                                    ;
  webPage2 += "                                      f = f + 1;"                                                 ;
  webPage2 += "                                      x1.setAttribute('value', TOFF_HR[k]);"                      ;
  webPage2 += "                                      x2.setAttribute('value', TOFF_MIN[k]);"                     ;
  webPage2 += "                                }"                                                                ;
  webPage2 += "                                z.appendChild(x1);"                                               ;
  webPage2 += "                                z.appendChild(x2);"                                               ;
  webPage2 += "                            } else { "                                                            ;
  webPage2 += "                                 var name = 'cell' + k + i;"                                      ;
  webPage2 += "                                 var TDAY_SET= T_DAY[i-2+k*7];"                                   ;
  webPage2 += "                                 x1.setAttribute('type', 'hidden');"                              ;
  webPage2 += "                                 x1.setAttribute('name', name);"                                  ;
  webPage2 += "                                 x2.setAttribute('type', 'checkbox');"                            ;
  webPage2 += "                                 x2.setAttribute('name', name);"                                  ;
  webPage2 += "                                 if (TDAY_SET == 'OFF') {"                                        ;
  webPage2 += "                                     x2.removeAttribute('checked');"                              ;
  webPage2 += "                                     x1.setAttribute('checked', 'checked');"                      ;
  webPage2 += "                                 }"                                                               ;
  webPage2 += "                                 if (TDAY_SET == 'ON') {"                                         ;
  webPage2 += "                                     x1.removeAttribute('checked');"                              ;
  webPage2 += "                                     x2.setAttribute('checked', 'checked');"                      ;
  webPage2 += "                                 }"                                                               ;
  webPage2 += "                                 z.appendChild(x1);"                                              ;
  webPage2 += "                                 z.appendChild(x2);"                                              ;
  webPage2 += "                            }"                                                                    ;
  webPage2 += "                            document.getElementById(mytr).appendChild(z);"                        ;
  webPage2 += "                        }"                                                                        ;
  webPage2 += "                    }"                                                                            ;

  //  webPage2 += "                   if  (document.getElementById('txt8').textContent == 'OFF') {"                  ;
  //  webPage2 += "                    setColor1();"                                                                 ;
  //  webPage2 += "                    }"                                                                            ;
  //  webPage2 += "                    if  (document.getElementById('txt8').textContent == 'ON') {"                  ;
  //  webPage2 += "                    setColor();"                                                                  ;
  //  webPage2 += "                    }"                                                                            ;

  webPage2 += "                }"                                                                                ;

  webPage2 += "              function BTN_COL() {"                                                               ;
  webPage2 += "                if  (document.getElementById('txt8').textContent == 'OFF') {"                     ;
  webPage2 += "                     setColor1(); "                                                               ;
  webPage2 += "                   }"                                                                             ;
  webPage2 += "                if  (document.getElementById('txt8').textContent == 'ON') {"                      ;
  webPage2 += "                     setColor(); "                                                                ;
  webPage2 += "                    }"                                                                            ;
  webPage2 += "                if  (document.getElementById('txt8').textContent == 'AUTO') {"                    ;
  webPage2 += "                     setColor2(); "                                                               ;
  webPage2 += "                    }"                                                                            ;
  webPage2 += "                }"                                                                                ;

  /*  webPage2 += "function updateTextInputMAX(val) {";
    webPage2 += "    document.getElementById('textInputMAX').value = val;}\n";
    webPage2 += "function updateTextInputMIN(val) {";
    webPage2 += "    document.getElementById('textInputMIN').value = val;}\n";
  */  webPage2 += "</script>\n";
  webPage2 += "</body>\n";
  webPage2 += "</html>\n";

  SettingsRemove();
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);
  File f2 = SPIFFS.open("/t.txt", "r");
  if (!f2) {
    tmax_state = "36";
    tmin_state = "24";
    TON_HR_ARR = "12,12,12,12,1,2";
    TOFF_HR_ARR = "13,13,13,13,11,12";
    TON_MIN_ARR = "31,32,33,34,13,14";
    TOFF_MIN_ARR = "41,42,43,44,23,23";
    T_DAY_ARR = "ON,ON,ON,ON,ON,OFF,OFF,ON,ON,ON,ON,ON,OFF,OFF,OFF,OFF,OFF,OFF,OFF,ON,ON,OFF,OFF,OFF,OFF,OFF,ON,ON,ON,ON,ON,ON,ON,OFF,OFF,ON,ON,ON,ON,ON,OFF,OFF";
    //    MY_SSID = "your ssid";
    //    MY_PWD = "your pwd";
    SettingsWrite();
    Serial.println("Test file open failed");
  }
  f2.close();

  SettingsRead();

  connectWifi();

  timeClient.begin();
  timeClient.update();
  GetSWtime();
  GetSWday();
  webpagebuild();

}



void loop() {

  gettemperature();
  checkOST();
  CURR_time = timeClient.getFormattedTime();
  CURR_time.remove(CURR_time.length() - 3);
  CURR_day = timeClient.getDay();

  // jakobs84
  for (int i = 0;  i < row ; i++) {


    if (CURR_time == TON[i] && TDAY[i][timeClient.getDay()] == "ON") {
      Serial.println("switch on the shit" );
      TIMER_SET = "ON";
    }
    if (CURR_time == TOFF[i] && TDAY[i][timeClient.getDay()] == "ON") {
      Serial.println("switch off the shit" );
      TIMER_SET = "OFF";
    }

  }
  /*
    // Turn the LED on (Note that LOW is the voltage level
    if (temp <= SwitchTemp2 && WFTMP == 1 && TIMER_SET == "ON") {
    WFTMP = 0;
    digitalWrite(LED_BUILTIN, LOW);
    digitalWrite(gpio2_pin, HIGH);
    }
    // Turn the LED off (Note that HIGH is the voltage level
    if ((temp >= SwitchTemp1 &&  WFTMP == 0 && TIMER_SET == "ON") or TIMER_SET == "OFF" ) {
    WFTMP = 1;
    digitalWrite(LED_BUILTIN, HIGH);
    digitalWrite(gpio2_pin, LOW);
    }
  */
  server.handleClient();
  /*
    count++ ;
    if (count == myPeriodic) {
      sendTeperatureTS(temp);
      count = 0;
    }
  */
  delay(1000);
}


void webpagebuild() {
  //  tmax_state.trim();
  //  tmin_state.trim();
  TON_HR_ARR.trim();
  TON_MIN_ARR.trim();
  TOFF_HR_ARR.trim();
  TOFF_HR_ARR.trim();
  TOFF_MIN_ARR.trim();
  T_DAY_ARR.trim();
  TIMER_SET.trim();

  webPage  = "";
  webPage += webPage1;
  webPage += "<p>OUT&nbsp;" + tempC + "&nbsp;<small>&deg;C</small> ";
  webPage += "&nbsp;IN&nbsp;&nbsp;" + tempC1 + "&nbsp;<small>&deg;C</small></p>";
  webPage += "<p hidden>";
  //  webPage += "<span id='txt1'>" + tmax_state + "</span>";
  //  webPage += "<span id='txt2'>" + tmin_state + "</span>";
  webPage += "<span id='txt3'>" + TON_HR_ARR + "</span>";
  webPage += "<span id='txt4'>" + TON_MIN_ARR + "</span>";
  webPage += "<span id='txt5'>" + TOFF_HR_ARR + "</span>";
  webPage += "<span id='txt6'>" + TOFF_MIN_ARR + "</span>";
  webPage += "<span id='txt7'>" + T_DAY_ARR + "</span>";
  webPage += "<span id='txt8'>" + TIMER_SET + "</span>";
  webPage += "</p>";
  webPage += webPage2;
}

void GetSWtime() {
  // jakobs84
  for ( int k = 0; k < row ; k++ ) {
    String st1 = getStringPartByNr(TON_HR_ARR, ',', k);
    String st2 = getStringPartByNr(TON_MIN_ARR, ',', k);
    String st3 = getStringPartByNr(TOFF_HR_ARR, ',', k);
    String st4 = getStringPartByNr(TOFF_MIN_ARR, ',', k);
    if (st1.length() == 1) {
      st1 = "0" + st1;
    }
    if (st2.length() == 1) {
      st2 = "0" + st2;
    }
    if (st3.length() == 1) {
      st3 = "0" + st3;
    }
    if (st4.length() == 1) {
      st4 = "0" + st4;
    }
    TON[k] = st1 + ":" + st2;
    TOFF[k] = st3 + ":" + st4;
    // jakobs84
    Serial.print(String(k) +" -  On time " + st1 + ":" + st2 + " Off time " + st3 + ":" + st4 +" ");

 /*   
    for ( int o = 0; o < 6 ; o++ ) {
     String st5 = getStringPartByNr(T_DAY_ARR, ',', k);
     Serial.print(String(st5) + " ");
     if (o == 6)
     Serial.println();
    }*/
    }
  
}

void GetSWday() {
  int n = 0;
  int j = 0;
  for ( int k = 0; k < (row * 7) ; k++ ) {
    String st5 = getStringPartByNr(T_DAY_ARR, ',', k);
    //the ifs were done to align the input of weekdays in ISO format with the weekday delivered from NTP library which is in US format
    if (n == 0) {
      TDAY[j][6] = st5;
    }

    if (0 > n < 6) {
      TDAY[j][n] = st5;
    }

    if (n == 6) {
      TDAY[j][0] = st5;
    }

    n++;
    if (n == 7) {
      n = 0;
      j++;
    }
 //   Serial.print(String(st5) + " " + String(k % 7) + " ");
 //   if (k>0 &&((k % 7) ==0))
 //   Serial.println();
  }


  //  }
}

// in case the settings file has to be renmoved

void SettingsRemove() {
  bool result = SPIFFS.begin();
  Serial.println("Delete SPIFFS opened: " + result);
  SPIFFS.remove("/t.txt");
  File f = SPIFFS.open("/t.txt", "w");
  if (!f) {
    Serial.println("Test file open failed");
  }
}


void SettingsWrite()
{
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);
  File f = SPIFFS.open("/t.txt", "w");
  // this opens the file "t.txt" in write-mode

  if (f) {
    //    Serial.println("File t.txt exist");
    TEMP1_SET = String("Temp1 = " + tmax_state);
    TEMP2_SET = String("Temp2 = " + tmin_state);
    TON_HR_set = String("TONHR = " + TON_HR_ARR);
    TOFF_HR_set = String("TOFFHR = " + TOFF_HR_ARR);
    TON_MIN_set = String("TONMIN = " + TON_MIN_ARR);
    TOFF_MIN_set = String("TOFFMIN = " + TOFF_MIN_ARR);
    T_DAY_set = String("TDAY = " + T_DAY_ARR);
    TON_HR_ARR.trim();
    //    Serial.println("stringlength "+ TON_HR_ARR);
    //    Serial.println(TON_HR_ARR.length());
    f.println (String("SSID = test"));
    f.println (String("PWD = test1"));
    f.println(TEMP1_SET);
    f.println(TEMP2_SET);
    f.println(TON_HR_set);
    f.println(TOFF_HR_set);
    f.println(TON_MIN_set);
    f.println(TOFF_MIN_set);
    f.println(T_DAY_set);
    Serial.println("write to file");

  }
  f.close();

}

void SettingsRead()
{
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);
  File f1 = SPIFFS.open("/t.txt", "r");
  // this opens the file "t.txt" in read-mode

  // we could open the file

  while (f1.available()) {
    //Lets read line by line from the file
    String line = f1.readStringUntil('\n');
    //    Serial.print("read line = ");
    //    Serial.println(line);

    //    if (line.substring(0, 5) == "Temp1")
    if (line.indexOf("Temp1") == 0)
    {
      //      Serial.println(line.substring(0, 5));
      line.remove(0, 8);
      //      line.remove(0, (line.length() - 3));
      Serial.println("Found Temperature1");
      SwitchTemp1 = line.toFloat();
      tmax_state = line;
      continue;

    }
    //    if (line.substring(0, 7) == "Temp2 =")
    if (line.indexOf("Temp2") == 0)
    {
      //      Serial.println(line.substring(0, 5));
      line.remove(0, 8);
      //      line.remove(0, (line.length() - 3));
      SwitchTemp2 = line.toFloat();
      tmin_state = line;
      //      Serial.println("Found Temperature2 ");
      continue;
    }

    if (line.indexOf("TONHR") == 0)
    {
      line.remove(0, 8);
      TON_HR_ARR = line;
      Serial.println("Found TONHR " + TON_HR_ARR);
      continue;
    }
    if (line.indexOf("TONMIN") == 0)
    {
      line.remove(0, 9);
      TON_MIN_ARR = line;
      //      Serial.println("Found TON_MIN " + TON_MIN_ARR);
      continue;
    }

    if (line.indexOf("TOFFHR") == 0)
    {

      line.remove(0, 9);
      TOFF_HR_ARR = line;
      //      Serial.println("Found TOFFHR " + TOFF_HR_ARR);
      continue;
    }
    if (line.indexOf("TOFFMIN") == 0)
    {

      line.remove(0, 10);
      TOFF_MIN_ARR = line;
      //      Serial.println("Found TOFF_MIN " + TOFF_MIN_ARR);
      continue;
    }
    if (line.indexOf("TDAY") == 0)
    {

      line.remove(0, 7);
      T_DAY_ARR = line;
      //      Serial.println("Found T_DAY " + T_DAY_ARR);
      continue;
    }



  }

  f1.close();
}


void connectWifi()
{
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  // Serial.print("Connecting to " + MY_SSID);
  //  char charBuf[50];
  //  char charBuf1[50];
  //  MY_SSID.toCharArray(charBuf, 50) ;
  //  MY_PWD.toCharArray(charBuf1, 50) ;
  //  WiFi.begin(charBuf, charBuf1);
  //  while (WiFi.status() != WL_CONNECTED) {
  //    delay(1000);
  //    Serial.print(".");
  //  }
  Serial.println("");
  Serial.print("Connected WIFI: ");
  // Print the IP address
  Serial.print(WiFi.SSID());
  Serial.print(" (");
  Serial.print(WiFi.RSSI());
  Serial.print("dB) - IP: ");
  Serial.println(WiFi.localIP());
  //webserver
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("Server started");
  }
  server.on("/", []() {

    for ( int p = 0; p < server.args(); p++ ) {

      if (server.argName(p) == "RESET_WIFI") {
        WiFiManager wifiManager1;
        wifiManager1.resetSettings();
        ESP.restart();

      }
      if (server.argName(p) == "ONBTN") {
        digitalWrite(gpio2_pin, HIGH);
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("ON");
        TIMER_SET = "ON" ;
      }
      if (server.argName(p) == "OFFBTN") {
        digitalWrite(gpio2_pin, LOW);
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("OFF");
        TIMER_SET = "OFF" ;
      }
      if (server.argName(p) == "AUTOBTN") {
        //        digitalWrite(gpio2_pin, LOW);
        //        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("AUTO");
        TIMER_SET = "AUTO" ;
      }
      if (server.argName(p) == "TempMAX") {
        tmax_state = server.arg(p);
      }
      if (server.argName(p) == "TempMIN") {
        tmin_state = server.arg(p);
      }
      if (server.argName(p) == "TH00") {
        TON_HR_ARR = server.arg(p);
      }
      if (server.argName(p) == "TH10") {
        TON_HR_ARR = TON_HR_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TH20") {
        TON_HR_ARR = TON_HR_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TH30") {
        TON_HR_ARR = TON_HR_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TM00") {
        TON_MIN_ARR = server.arg(p);
      }
      if (server.argName(p) == "TM10") {
        TON_MIN_ARR = TON_MIN_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TM20") {
        TON_MIN_ARR = TON_MIN_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TM30") {
        TON_MIN_ARR = TON_MIN_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TH01") {
        TOFF_HR_ARR = server.arg(p);
      }
      if (server.argName(p) == "TH11") {
        TOFF_HR_ARR = TOFF_HR_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TH21") {
        TOFF_HR_ARR = TOFF_HR_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TH31") {
        TOFF_HR_ARR = TOFF_HR_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TM01") {
        TOFF_MIN_ARR = server.arg(p);
      }
      if (server.argName(p) == "TM11") {
        TOFF_MIN_ARR = TOFF_MIN_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TM21") {
        TOFF_MIN_ARR = TOFF_MIN_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "TM31") {
        TOFF_MIN_ARR = TOFF_MIN_ARR + "," + server.arg(p);
      }
      if (server.argName(p) == "cell02" and server.argName(p + 1) == "cell02") {
        T_DAY_ARR = "ON";
        ++p;
      }
      if (server.argName(p) == "cell02" and T_DAY_ARR != "ON") {
        T_DAY_ARR = "OFF";
      }
      if (server.argName(p).substring(0, 4) == "cell" and server.argName(p) != "cell02") {
        if (server.argName(p) == server.argName(p + 1)) {
          T_DAY_ARR = T_DAY_ARR + "," + "ON";
          ++p;
        }
        else {
          T_DAY_ARR =  T_DAY_ARR + "," + "OFF";
        }
      }

    }
    SettingsWrite();
    SettingsRead();
    webpagebuild();
    GetSWday();
    GetSWtime();
    server.send(200, "text/html", webPage);
  });

  server.begin();
  Serial.println("HTTP server started");
}//end connect


// string conversion
String convertTString (float Ftemp) {
  return String(dtostrf(Ftemp, 2, 2, temperatureString));
}
// get temperature from sensor
void gettemperature() {
  DS18B20.requestTemperatures();
  temp = DS18B20.getTempCByIndex(0);
  temp1 = DS18B20.getTempCByIndex(1);
  tempC = convertTString(temp);
  tempC1 = convertTString(temp1);
}
// check time
void checkOST() {
  currentMillis = millis();//actual time in ms
  //time verification logic
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;    // set time to actual
    //    printf("Time Epoch: %d: ", timeClient.getEpochTime());
    //    Serial.println(timeClient.getFormattedTime());
    timeClient.forceUpdate();
  }
}
////some string stuff from here:https://github.com/BenTommyE/Arduino_getStringPartByNr usage: getStringPartByNr(string, 'delimiter', position)
String getStringPartByNr(String data, char separator, int index) {
  // spliting a string and return the part nr index
  // split by separator
  int stringData = 0;        //variable to count data part nr
  String dataPart = "";      //variable to hole the return text
  for (int i = 0; i < data.length(); i++) { //Walk through the text one letter at a time
    if (data[i] == separator) {
      //Count the number of times separator character appears in the text
      stringData++;
    } else if (stringData == index) {
      //get the text when separator is the rignt one
      dataPart.concat(data[i]);

    } else if (stringData > index) {
      //return text and stop if the next separator appears - to save CPU-time
      return dataPart;
      break;

    }

  }
  //return text if this is the last part
  return dataPart;
}



// send temperature to thingspeak
void sendTeperatureTS(float temp) {
  WiFiClient client;
  if (client.connect(IOTserver, 80)) { // use ip 184.106.153.149 or api.thingspeak.com
    Serial.println("WiFi Client connected ");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    String postStr = apiKey;
    postStr += "&field2=";
    postStr += String(temp);
    postStr += "\r\n\r\n";
    Serial.println(postStr);
    client.print("GET /update?api_key=" + apiKey + "&field2=" + String(temp) + "\r\n");
    client.print("Host: api.thingspeak.com\r\n");
    client.print("Connection: close\r\n");
    client.print("Accept: */*\r\n");
    client.print("User-Agent: Mozilla/4.0 (compatible; esp8266 Lua; Windows NT 5.1)\r\n");
    client.print("\r\n");
    delay(1000);
  }//end if
  sent++;
  client.stop();
}//end send


