#include "Arduino.h"
#include <Adafruit_Arcada.h>
#include "AverageTemp.h"
#include "arcada_thermometer.h"
#include "audio.h"

//---- brightness ----

int led_brightness_table[LBR_SIZE] = {0, 14, 29,  59,  123,   255};

//---- colors ----

uint32_t PX_RED = arcada.pixels.Color(1,0,0);
uint32_t PX_YELLOW = arcada.pixels.Color(1,1,0);
uint32_t PX_GREEN = arcada.pixels.Color(0,1,0);
uint32_t PX_CYAN = arcada.pixels.Color(0,1,1);
uint32_t PX_BLUE = arcada.pixels.Color(0,0,1);
uint32_t PX_BLACK = arcada.pixels.Color(0,0,0);

//---- LED count memory ----

int oldCount = 0;

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

//---- play_tune ----
void play_tune(const uint8_t *audio, uint32_t audio_length) {
  uint32_t t;
  uint32_t prior, usec = 1000000L / SAMPLE_RATE;
  analogWriteResolution(8);
  for (uint32_t i=0; i<audio_length; i++) {
    while((t = micros()) - prior < usec);
    analogWrite(A0, (uint16_t)audio[i] / 8);
    analogWrite(A1, (uint16_t)audio[i] / 8);
    prior = t;
  }
}

//---- goodChange ----
bool goodChange() {
  float delta = avgTemp.temp_disp - (t_set / 2.0);
  return delta > 0.0 && tempChange < 0.0 || delta < 0.0 && tempChange > 0.0;
}

//---- beepIfNeeded ----
void beepIfNeeded(int count) {
  if (count == 0) {
    // hide the counter
    bellCountdown = -1;
    oldCount = 0;
  } else if (bellCountdown <= 0 || count > 0 && count > oldCount || count < 0 && count < oldCount) {
    // the bell counter has finished or a new LED is on
    if (sound && ! goodChange()) {
      // beep
      arcada.enableSpeaker(true);
      play_tune(audio, sizeof(audio));
      arcada.enableSpeaker(false);
    }
    bellCountdown = intervals[time_interval];
    oldCount = count;
  }
  if (changeCountdown == 0) {
    // hide the change counter and the tempChange mark
    tempChange = 0.0;
    changeCountdown = -1;
  }
}
