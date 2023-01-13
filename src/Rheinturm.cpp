#include "Rheinturm.h"
/*
    WiFi-Settings: https://github.com/Juerd/ESP-WiFiSettings
    NTP-Server: https://www.pool.ntp.org/zone/europe
    time zone settings: (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
    See https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv for Timezone codes for your region
    https://github.com/espressif/arduino-esp32/tree/master/libraries/Preferences
*/



#include <SPIFFS.h>
#include <WiFiSettings.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <Preferences.h>
#include <WebServer.h>


WebServer server(80);

Colors clock_colors;

Preferences preferences;

CRGB ClockLeds[NUM_PIXELS_CLOCK];
CRGB AuxLeds[NUM_PIXELS_AUX];


Uhrzeit uhrzeit;




int Modus = 1;
int Touch_Value = 0;
bool Touched = 0;
unsigned long Old_Millis = 0;
int Long_Touch = 0;
unsigned long Press_Time;


const char* NTP_SERVER = "de.pool.ntp.org";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";






// Start ArduinoOTA via WiFiSettings with the same hostname and password
void setup_ota() {
  ArduinoOTA.setHostname(WiFiSettings.hostname.c_str());
  ArduinoOTA.setPassword(WiFiSettings.password.c_str());
  ArduinoOTA.begin();
}

void setup() {

  Serial.begin(115200);


  SPIFFS.begin(true);  // Will format on the first run after failing to mount

  // Force WPA secured WiFi for the software access point.
  // Because OTA is remote code execution (RCE) by definition, the password
  // should be kept secret. By default, WiFiSettings will become an insecure
  // WiFi access point and happily tell anyone the password. The password
  // will instead be provided on the Serial connection, which is a bit safer.
  WiFiSettings.secure = true;

  // Set callbacks to start OTA when the portal is active
  WiFiSettings.onPortal = []() {
    setup_ota();
  };
  WiFiSettings.onPortalWaitLoop = []() {
    ArduinoOTA.handle();
  };

  // Use stored credentials to connect to your WiFi access point.
  // If no credentials are stored or if the access point is out of reach,
  // an access point will be started with a captive portal to configure WiFi.
  WiFiSettings.connect();

  Serial.print("Password: ");
  Serial.println(WiFiSettings.password);

  setup_ota();  // If you also want the OTA during regular execution

  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);

  if (getNTPtime(10, &uhrzeit)) {  // wait up to 10sec to sync
  } else {
    //Serial.println("Time not set");
    ESP.restart();
  }

  // Einstellungen laden....
  preferences.begin("config", false);
  // preferences.clear(); // remove all preferences in namespace myfile
  // preferences.remove("varname");// remove varname in the namespace
  // preferences.putUInt("CCSE", 0xFFFF00);
  // preferences.putFloat("param", param);
  delay(50);

  clock_colors.CLOCK_COLOR_SEKUNDEN_EINER  = preferences.getUInt("CCSE", 0xFFFF00);
  clock_colors.CLOCK_COLOR_SEKUNDEN_ZEHNER = preferences.getUInt("CCSZ", 0xFFFF00);
  clock_colors.CLOCK_COLOR_MINUTEN_EINER   = preferences.getUInt("CCME", 0xFFFF00);
  clock_colors.CLOCK_COLOR_MINUTEN_ZEHNER  = preferences.getUInt("CCMZ", 0xFFFF00);
  clock_colors.CLOCK_COLOR_STUNDEN_EINER   = preferences.getUInt("CCHE", 0xFFFF00);
  clock_colors.CLOCK_COLOR_STUNDEN_ZEHNER  = preferences.getUInt("CCHZ", 0xFFFF00);
  clock_colors.CLOCK_COLOR_OFF             = preferences.getUInt("CCO" , 0x000000);




  preferences.end();
  // Einstellungen ende


  // Webserver...
  server.on("/", handle_OnConnect);
  server.begin();
  Serial.println("HTTP server started");
  // Webserver ende


  FastLED.addLeds<PL9823, LED_PIN_CLOCK>(ClockLeds, NUM_PIXELS_CLOCK);
  FastLED.addLeds<PL9823, LED_PIN_AUX>(AuxLeds, NUM_PIXELS_AUX);
  FastLED.setMaxPowerInMilliWatts(1000);
  FastLED.setBrightness(  BRIGHTNESS );
  FastLED.clear();
  FastLED.show();
}

void loop() {
  ArduinoOTA.handle();  // If you also want the OTA during regular execution
  server.handleClient();
  // Your loop code here
  getNTPtime(1, &uhrzeit);
  display_clock(&(ClockLeds[0]), &uhrzeit, &clock_colors);
  //display_aux();

  FastLED.show();

  Touch_Value = touchRead(Touch_PIN);
  if (Touch_Value < Touch_Threshold && Touched == 0) {
    Old_Millis = millis();
    Touched = 1;
  } else if (Touch_Value > Touch_Threshold && Touched == 1) {
    Press_Time  = millis() - Old_Millis;
    if (Press_Time > 200 && Press_Time < 1500) {
      Modus++;
      Touched = 0;
    } else if (Press_Time > 1500) {
      Long_Touch++;
      Touched = 0;
    } else {
      Touched = 0;
    }
  }
  if (Modus == 3 ) Modus = 1;


  //Serial.println(preferences.freeEntries());



#ifdef DEBUG
  //Serial.print(Modus);
  //Serial.print(" ");
  //Serial.print(Long_Touch);
  //Serial.print(" ");
  //Serial.print(Touched);
  //Serial.print(" ");
  //Serial.print(Press_Time);
  //Serial.print(" ");
  //Serial.println(Touch_Value);
#endif
}







void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(1,1));
}



String SendHTML(uint8_t led1stat,uint8_t led2stat){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Rheinturm</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>Rheinturm</h1>\n";
  ptr +="<h3>Konfiguration</h3>\n";
  ptr +="<br>";
  char myHex[10] = "";
  ptr += "CLOCK_COLOR_SEKUNDEN_EINER: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_SEKUNDEN_EINER,myHex,16);
  ptr += "<br>";
  ptr += "CLOCK_COLOR_SEKUNDEN_ZEHNER: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_SEKUNDEN_ZEHNER,myHex,16);
  ptr += "<br>";
  ptr += "CLOCK_COLOR_MINUTEN_EINER: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_MINUTEN_EINER,myHex,16);
  ptr += "<br>";
  ptr += "CLOCK_COLOR_MINUTEN_ZEHNER: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_MINUTEN_ZEHNER,myHex,16);
  ptr += "<br>";
  ptr += "CLOCK_COLOR_STUNDEN_EINER: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_STUNDEN_EINER,myHex,16);
  ptr += "<br>";
  ptr += "CLOCK_COLOR_STUNDEN_ZEHNER: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_STUNDEN_ZEHNER,myHex,16);
  ptr += "<br>";
  ptr += "CLOCK_COLOR_OFF: ";
  ptr += itoa(clock_colors.CLOCK_COLOR_OFF,myHex,16);
  ptr += "<br>";



  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}