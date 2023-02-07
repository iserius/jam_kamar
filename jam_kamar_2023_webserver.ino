/*
-- update 30 jan 2023
-- library arduino json by benoit blanchon
-- NTPClient and install the library by Fabrice Weinber 
pilih json5 atau json6
 * add OTA
  ESP-01 pinout from top:
  
  GND    GP2 GP0 RX/GP3
  TX/GP1 CH  RST VCC

  MAX7219
  ESP-1 from rear
  Re Br Or Ye
  Gr -- -- --

  USB to Serial programming
  ESP-1 from rear, FF to GND, RR to GND before upload
  Gr FF -- Bl
  Wh -- RR Vi

  GPIO 2 - DataIn
  GPIO 1 - LOAD/CS
  GPIO 0 - CLK

  ------------------------
  NodeMCU 1.0 pinout:

  D8 - DataIn
  D7 - LOAD/CS
  D6 - CLK
  
*/
#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
 // #include <ESP8266HTTPClient.h>
  //#include <WiFiClient.h>
  //#include <ESP8266WebServer.h>
  #include <ESPAsyncTCP.h>
  #include <FS.h>
#endif

#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include "Arduino.h"
#include <ArduinoJson.h>
//#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

AsyncWebServer server(80);
// =======================================================================
#define MAX_DIGITS 16
byte dig[MAX_DIGITS]={0};
byte digold[MAX_DIGITS]={0};
byte digtrans[MAX_DIGITS]={0};
int updCnt = 0;
int dots = 0;
long dotTime = 0;
long clkTime = 0;
int dx=0;
int dy=0;
byte del=0;
int h,m,s;
// =======================================================================
// =======================================================================

float utcOffset = 7;
long localEpoc = 0;
long localMillisAtUpdate = 0;
String weatherString;
String pesan = "message in a bottle........";
int selang;
// =======================================================================
#ifndef STASSID
//#define STASSID "iSerius_TailQuarter"
#define STASSID "WaiFaii"
#define STAPSK  "bismillah"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;
int retset = 0;
int shownow = 0;
WiFiClient client;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
//String weekDays[7]={"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
String weekDays[7]={"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

//Month names
//String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
String months[12]={"Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember"};


// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;
String date;
String tanggal;
String hari;
String bulan;
String tahun;
String result;
String hariini;
#define NUM_MAX 16
//#define NUM_MAX 5

// for ESP-01 module
//#define DIN_PIN 2 // D4
//#define CS_PIN  3 // D9/RX
//#define CLK_PIN 0 // D3

// for NodeMCU 1.0
#define DIN_PIN 15  // D8
#define CS_PIN  13  // D7
#define CLK_PIN 12  // D6

#include "max7219.h"
#include "fonts.h"

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";
const char* PARAM_INPUT_4 = "input4";

// HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>masukan Pesan</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    Message: <input type="text" name="input1"><input type="checkbox" id="shownow" name="input4" value="1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    Delay: <input type="number" name="input2">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    <input type="checkbox" name="input3" value="1">
    <input type="submit" value="Reset">
  </form>
</body></html>)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


void getTime()
{
  timeClient.update();
  time_t epochTime = timeClient.getEpochTime();
  
  Serial.print("Epoch Time: ");
  Serial.println(epochTime);
  
  String formattedTime = timeClient.getFormattedTime();
  Serial.print("Formatted Time: ");
  Serial.println(formattedTime);  

  int currentHour = timeClient.getHours();
  Serial.print("Hour: ");
  Serial.println(currentHour);  

  int currentMinute = timeClient.getMinutes();
  Serial.print("Minutes: ");
  Serial.println(currentMinute); 
   
  int currentSecond = timeClient.getSeconds();
  Serial.print("Seconds: ");
  Serial.println(currentSecond);  

  String weekDay = weekDays[timeClient.getDay()];
  hari = weekDay;
  Serial.print("Week Day: ");
  Serial.println(weekDay);    

  //Get a time structure
  struct tm *ptm = gmtime ((time_t *)&epochTime); 

  int monthDay = ptm->tm_mday;
  tanggal = monthDay;
  Serial.print("Month day: ");
  Serial.println(monthDay);

  int currentMonth = ptm->tm_mon+1;
  Serial.print("Month: ");
  Serial.println(currentMonth);

  String currentMonthName = months[currentMonth-1];
  bulan = currentMonthName;
  Serial.print("Month name: ");
  Serial.println(currentMonthName);

  int currentYear = ptm->tm_year+1900;
  tahun = currentYear;
  Serial.print("Year: ");
  Serial.println(currentYear);

  //Print complete date:
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.print("Current date: ");
  Serial.println(currentDate);

  Serial.println("");

      ///// get time ////////
      h = timeClient.getHours();
      m = timeClient.getMinutes();
      s = timeClient.getSeconds();

    hariini = String(hari) + ", " + String(tanggal) + " " + String(bulan)+ " " + String(tahun);
      
      Serial.print("UTC time ");
      Serial.print(h);
      Serial.print(":");
      Serial.print(m);
      Serial.print(":");
      Serial.println(s);
      Serial.print("WIB time ");
      Serial.print((h+7));
      Serial.print(":");
      Serial.print(m);
      Serial.print(":");
      Serial.println(s);
      Serial.println(hariini);
    

      localMillisAtUpdate = millis();
      localEpoc = (h * 60 * 60 + m * 60 + s);
}


void updateTime()
{
  long curEpoch = localEpoc + ((millis() - localMillisAtUpdate) / 1000);
 // long epoch = round(curEpoch + 3600 * utcOffset + 86400L) % 86400L;
 long epoch = round(curEpoch + 3600 * utcOffset + 86400L);
  h = ((epoch  % 86400L) / 3600) % 24;
  m = (epoch % 3600) / 60;
  s = epoch % 60;
}

void setup() 
{
  Serial.begin(115200);
  initMAX7219();
  sendCmdAll(CMD_SHUTDOWN,1);
  sendCmdAll(CMD_INTENSITY,0);
  Serial.println("Booting");
  printStringWithShift(".Alma Katya..",5);
  printStringWithShift("....... .. ......",5);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    printStringWithShift("Connection Failed! Rebooting...",10);
    delay(4000);
    ESP.restart();
  }

  // Initialize a NTPClient to get time
  timeClient.begin();
  Serial.println("timeClient started");
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
 // timeClient.setTimeOffset(25200);

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      pesan = inputMessage;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
     //c selang = inputMessage;
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      retset = inputMessage.toInt();
    }
     else if (request->hasParam(PARAM_INPUT_4)) {
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      shownow = inputMessage.toInt();
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
  
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
    printStringWithShift("        Updating Firmware ",15);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    char inih =("Progress:%u%%\r", (progress / (total / 100)));
    printCharWithShift(inih,1);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
      printStringWithShift("Auth Failed",5);
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
      printStringWithShift("Begin Failed",5);
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
      printStringWithShift("Connect Failed",5);
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
      printStringWithShift("Receive Failed",5);
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
      printStringWithShift("End Failed",5);
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  printStringWithShift("IP:..",5);
  printStringWithShift(WiFi.localIP().toString().c_str(),5);
  printStringWithShift("............... .... ",5);
}

void loop()
{
  ArduinoOTA.handle();

 timeClient.update();
  
  if(updCnt<=0) { // every 10 scrolls, ~450s=7.5m
    updCnt = 30;// kalau 10 = 7.5m
    //Serial.println("Getting data ...");
    //printStringWithShift("        SyncData..",15);
    //getWeatherData();
    getTime();
    //Serial.println("Data loaded");
    //printStringWithShift("       Data loaded",15);
    clkTime = millis();
  }
  if(retset >=1){ ESP.restart();}
  if(shownow >=1){ 
      printStringWithShift(pesan.c_str(),7); 
      printStringWithShift("             ",7);
      shownow = 0;}
      
 if((h==12)&& (m==00)&&(s==00)){ ESP.restart();}
//if(millis()-clkTime > 35000 && !del && dots) { // clock for 15s, then scrolls for about 30s
//if((millis()-clkTime > 3000 && !del && dots)&& (h>6)) { // clock for 15s, then scrolls for about 30s
   
  if (millis()-clkTime > 60000 && !del && dots) { // clock for 15s, then scrolls for about 30s
    //string dekdek = hari.c_str()
    //string dikdik = "     "
    //string dukduk
    printStringWithShift("     ",7);
    //printStringWithShift(hari.c_str(),7);
    //printStringWithShift(tanggal.c_str(),7);
    //printStringWithShift(bulan.c_str(),7);
    //printStringWithShift(tahun.c_str(),7);
    printStringWithShift(hariini.c_str(),7);
    //printStringWithShift(date.c_str(),5);
    printStringWithShift("          ",7);
   // printStringWithShift(currencyRates.c_str(),35);
    //printStringWithShift(weatherString.c_str(),7);
    printStringWithShift(pesan.c_str(),7);
    printStringWithShift("             ",7);
    updCnt--;
    clkTime = millis();
  }
  if(millis()-dotTime > 470) {  // dot blink speed
    dotTime = millis();
    dots = !dots;
  }
  updateTime();
  showAnimClock();
 // printime();
  //showSimpleClock(); 
}

// =======================================================================

void showSimpleClock()
{
  dx=dy=0;
  clr();
  showDigit(h/10,  0, dig6x8);
  showDigit(h%10,  8, dig6x8);
  showDigit(m/10, 17, dig6x8);
  showDigit(m%10, 25, dig6x8);
  showDigit(s/10, 34, dig6x8);
  showDigit(s%10, 42, dig6x8);
  setCol(15,dots ? B00100100 : 0);
  setCol(32,dots ? B00100100 : 0);
  refreshAll();
}

// =======================================================================

void showAnimClock()
{
  byte digPos[6]={0,8,17,25,34,42};
  int digHt = 12;
  int num = 6; 
  int i;
  if(del==0) {
    del = digHt;
    for(i=0; i<num; i++) digold[i] = dig[i];
    dig[0] = h/10 ? h/10 : 10;
    dig[1] = h%10;
    dig[2] = m/10;
    dig[3] = m%10;
    dig[4] = s/10;
    dig[5] = s%10;
    for(i=0; i<num; i++)  digtrans[i] = (dig[i]==digold[i]) ? 0 : digHt;
  } else
    del--;
  
  clr();
  for(i=0; i<num; i++) {
    if(digtrans[i]==0) {
      dy=0;
      showDigit(dig[i], digPos[i], dig6x8);
    } else {
      dy = digHt-digtrans[i];
      showDigit(digold[i], digPos[i], dig6x8);
      dy = -digtrans[i];
      showDigit(dig[i], digPos[i], dig6x8);
      digtrans[i]--;
    }
  }
  dy=0;
  setCol(15,dots ? B00100100 : 0);
  setCol(32,dots ? B00100100 : 0);
  refreshAll();
  delay(30);
}

// =======================================================================

void showDigit(char ch, int col, const uint8_t *data)
{
  if(dy<-8 | dy>8) return;
  int len = pgm_read_byte(data);
  int w = pgm_read_byte(data + 1 + ch * len);
  col += dx;
  for (int i = 0; i < w; i++)
    if(col+i>=0 && col+i<8*NUM_MAX) {
      byte v = pgm_read_byte(data + 1 + ch * len + 1 + i);
      if(!dy) scr[col + i] = v; else scr[col + i] |= dy>0 ? v>>dy : v<<-dy;
    }
}

// =======================================================================

void setCol(int col, byte v)
{
  if(dy<-8 | dy>8) return;
  col += dx;
  if(col>=0 && col<8*NUM_MAX)
    if(!dy) scr[col] = v; else scr[col] |= dy>0 ? v>>dy : v<<-dy;
}

// =======================================================================

int showChar(char ch, const uint8_t *data)
{
  int len = pgm_read_byte(data);
  int i,w = pgm_read_byte(data + 1 + ch * len);
  for (i = 0; i < w; i++)
    scr[NUM_MAX*8 + i] = pgm_read_byte(data + 1 + ch * len + 1 + i);
  scr[NUM_MAX*8 + i] = 0;
  return w;
}


// =======================================================================

void printCharWithShift(unsigned char c, int shiftDelay) {
  if (c < ' ' || c > '~'+25) return;
  c -= 32;
  int w = showChar(c, font);
  for (int i=0; i<w+1; i++) {
    delay(shiftDelay);
    scrollLeft();
    refreshAll();
  }
}

// =======================================================================

void printStringWithShift(const char* s, int shiftDelay){
  while (*s) {
    printCharWithShift(*s, shiftDelay);
    s++;
  }
}

// =======================================================================

const char weatherHost[] = "https://www.bmkg.go.id/cuaca/prakiraan-cuaca.bmkg?Kota=Bandung&AreaID=501212&Prov=35";

void getWeatherData()
{
  Serial.print("connecting to "); Serial.println(weatherHost);
  printStringWithShift("        Connecting to weatherHost..",15);
  if (client.connect(weatherHost, 80)) {
  Serial.println("connection to google failed");
    printStringWithShift("        Connection to google failed..",5);
    return;
    
  }   client.print(String("GET / HTTP/1.1\r\n") +
               String("Host: https://www.bmkg.go.id/cuaca/prakiraan-cuaca.bmkg?Kota=Bandung&AreaID=501212&Prov=35\r\n") +
               String("Connection: close\r\n\r\n"));
  int repeatCounter = 0;
  while (!client.available() && repeatCounter < 10) {
    delay(500);
    Serial.println(".");
    repeatCounter++;
  
  }
  String line;
  client.setNoDelay(false);
  while(client.connected() && client.available()) {
    line = client.readStringUntil('\n');
    //line.toUpperCase();
    if (line.startsWith("pada ")) {


    weatherString  = line.substring(10,30);
    }
  }
    Serial.println(weatherString);
    client.stop();




 
}

   



// =======================================================================
