#include "Arduino.h"
#include <Adafruit_Arcada.h>
#include "AverageTemp.h"
#include "arcada_thermometer.h"

//---- brightness ----

int led_brightness_table[LBR_SIZE] = {0, 14, 29,  59,  123,   255};

//---- colors ----

uint32_t PX_RED = arcada.pixels.Color(1,0,0);
uint32_t PX_YELLOW = arcada.pixels.Color(1,1,0);
uint32_t PX_GREEN = arcada.pixels.Color(0,1,0);
uint32_t PX_CYAN = arcada.pixels.Color(0,1,1);
uint32_t PX_BLUE = arcada.pixels.Color(0,0,1);
uint32_t PX_BLACK = arcada.pixels.Color(0,0,0);

//---- updatePixels ----
int updatePixels() {
  float delta = avgTemp.temp_disp - (t_set / 2.0);
  uint32_t color = (delta > 0
      ? (tempChange >= 0.0 ? PX_RED : PX_YELLOW)
      : (tempChange <= 0.0 ? PX_BLUE : PX_GREEN)
      ) * led_brightness_table[led_brightness];
  int count = delta / 0.5;
  int absCount = abs(count);
  arcada.pixels.setPixelColor(2, absCount > 0 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(3, absCount > 1 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(1, absCount > 2 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(4, absCount > 3 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(0, absCount > 4 ? color : PX_BLACK);
  arcada.pixels.show();
  return count;
}
