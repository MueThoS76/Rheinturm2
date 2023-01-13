#include "Rheinturm.h"


void display_clock(CRGB* ClockLeds, Uhrzeit* uhrzeit, Colors* clock_colors) {
  for (int i = LED_SEKUNDEN_EINER_MIN; i <= LED_SEKUNDEN_EINER_MAX; i++) {
    if (i - LED_SEKUNDEN_EINER_MIN < uhrzeit->sekunden.einer) {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_SEKUNDEN_EINER;
    } else {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_OFF;
    }
  }

  for (int i = LED_SEKUNDEN_ZEHNER_MIN; i <= LED_SEKUNDEN_ZEHNER_MAX; i++) {
    if (i - LED_SEKUNDEN_ZEHNER_MIN < uhrzeit->sekunden.zehner) {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_SEKUNDEN_ZEHNER;
    } else {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_OFF;
    }
  }

  for (int i = LED_MINUTEN_EINER_MIN; i <= LED_MINUTEN_EINER_MAX; i++) {
    if (i - LED_MINUTEN_EINER_MIN < uhrzeit->minuten.einer) {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_MINUTEN_EINER;
    } else {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_OFF;
    }
  }
  for (int i = LED_MINUTEN_ZEHNER_MIN; i <= LED_MINUTEN_ZEHNER_MAX; i++) {
    if (i - LED_MINUTEN_ZEHNER_MIN < uhrzeit->minuten.zehner) {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_MINUTEN_ZEHNER;
    } else {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_OFF;
    }
  }
  for (int i = LED_STUNDEN_EINER_MIN; i <= LED_STUNDEN_EINER_MAX; i++) {
    if (i - LED_STUNDEN_EINER_MIN < uhrzeit->stunden.einer) {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_STUNDEN_EINER;
    } else {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_OFF;
    }
  }
  for (int i = LED_STUNDEN_ZEHNER_MIN; i <= LED_STUNDEN_ZEHNER_MAX; i++) {
    if (i - LED_STUNDEN_ZEHNER_MIN < uhrzeit->stunden.zehner) {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_STUNDEN_ZEHNER;
    } else {
      ClockLeds[i] = clock_colors->CLOCK_COLOR_OFF;
    }
  }

}
