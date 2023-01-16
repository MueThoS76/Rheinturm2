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
  server.on("/", handleRoot);
  server.on("/getdata",sendDataToBrowser);
  server.on("/senddata",setDataFromBrowser);
  server.on("/savedata",saveDataFromBrowser);
  server.onNotFound(handleWebRequests);
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




void handleRoot(){
  server.sendHeader("Location", "/index.html",true);   //Redirect to our html web page
  server.send(302, "text/plane","");
}

void sendDataToBrowser(){
  // Sende Alle Werte als JSON zum Browser
  server.sendContent(R"([
    {
        "CLOCK_COLOR_SEKUNDEN_EINER1":{"type":"color", "value": "#ff0000"},
        "CLOCK_COLOR_SEKUNDEN_ZEHNER2":{"type":"color", "value": "#00ff00"},
        "CLOCK_COLOR_MINUTEN_EINER3":{"type":"color", "value": "#0000FF"},
        "CLOCK_COLOR_MINUTEN_ZEHNER4":{"type":"color", "value": "#ffff00"},
        "CLOCK_COLOR_STUNDEN_EINER5":{"type":"color", "value": "#ff00ff"},
        "CLOCK_COLOR_STUNDEN_ZEHNER6":{"type":"color", "value": "#00ffff"},
        "CLOCK_COLOR_OFF7":{"type":"color", "value": "#ffFFFF"},
        "Helligkeit8":{"type":"range","min":"1", "max":"255", "value": "125"},
        "wecker9":{"type":"time", "value": "07:01"},
        "wecker_activ10":{"type":"checkbox", "checked": "false"},
        "MODUS11":{"type":"radio", "options":[{"value": "Uhrzeit","checked":"false"}, {"value": "Party","checked":"true"}, {"value": "test","checked":"false"}]}
    }
])");
};

void setDataFromBrowser(){
  // Setze die Werte die vom Browser kommen als aktive parameter
};
void saveDataFromBrowser(){
  // Setze die Werte die vom Browser kommen als aktive parameter
  // und speichere alles auch im eeprom
};

void handleWebRequests(){
      if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
  };

bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }

  dataFile.close();
  return true;
}
