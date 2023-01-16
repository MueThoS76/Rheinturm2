#include <Arduino.h>
#include <FastLED.h>

struct ziffern {
  byte einer;
  byte zehner;
};

struct Uhrzeit {
    ziffern sekunden;
    ziffern minuten;
    ziffern stunden;
};

struct Colors
{
    uint CLOCK_COLOR_SEKUNDEN_EINER;
    uint CLOCK_COLOR_SEKUNDEN_ZEHNER;
    uint CLOCK_COLOR_MINUTEN_EINER;
    uint CLOCK_COLOR_MINUTEN_ZEHNER;
    uint CLOCK_COLOR_STUNDEN_EINER;
    uint CLOCK_COLOR_STUNDEN_ZEHNER;
    uint CLOCK_COLOR_OFF;

};



#define Touch_PIN 14 // Touchsensor
#define Touch_Threshold 20
//#define DEBUG 1


bool getNTPtime(int sec, Uhrzeit* uhrzeit);
void display_clock(CRGB* ClockLeds, Uhrzeit* uhrzeit, Colors* clock_colors);
void display_aux();
String SendHTML(uint8_t led1stat,uint8_t led2stat);

void handleRoot();
void sendDataToBrowser();
void setDataFromBrowser();
void saveDataFromBrowser();
void handleWebRequests();
bool loadFromSpiffs(String path);




#define NUM_PIXELS_CLOCK 39 // Anzahl LEDs
#define LED_PIN_CLOCK    16 // LED Pin
#define NUM_PIXELS_AUX   3  // Anzahl LEDs
#define LED_PIN_AUX      17 // LED Pin
#define BRIGHTNESS  10

#define LED_SEKUNDEN_EINER_MIN 0
#define LED_SEKUNDEN_EINER_MAX 8
#define LED_SEKUNDEN_ZEHNER_MIN 9
#define LED_SEKUNDEN_ZEHNER_MAX 13

#define LED_MINUTEN_EINER_MIN 14
#define LED_MINUTEN_EINER_MAX 22
#define LED_MINUTEN_ZEHNER_MIN 23
#define LED_MINUTEN_ZEHNER_MAX 27

#define LED_STUNDEN_EINER_MIN 28
#define LED_STUNDEN_EINER_MAX 36
#define LED_STUNDEN_ZEHNER_MIN 37
#define LED_STUNDEN_ZEHNER_MAX 38

