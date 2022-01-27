
#include <Adafruit_Arcada.h>
#include "audio.h"
#include "AverageTemp.h"

#include "arcada_thermometer.h"

// DEBUG flag -> wait for Serial
#define DEBUG 0

//---- arcada ----
Adafruit_Arcada arcada; // cp437

//---- data tables and constants ---
int intervals[INTERVALS_SIZE] = {5, 15, 30, 60, 120, 180, 300};

//---- data to display ----
int t_set = 43; // in half degrees Celsius
AverageTemp avgTemp;
float tempChange;

AverageTemp voltage;
uint16_t light = 0;
int time_interval = 3;
int lcd_brightness = 2;
bool lcd_auto = true;
int led_brightness = 3;
bool led_auto = true;
bool sound = true;
volatile int countdown = intervals[time_interval];

//---- timer callback ----
// This function is called every second
volatile bool measure = true;
void timercallback() {
  if (countdown > 0) {
    countdown--;
  }
  analogWrite(13, 1); // weak red light on the LED
  measure = true;
}

//---- setup ----
void setup() {
  if (DEBUG) {
    while (!Serial);
  }
  Serial.begin(115200);

  Serial.println("Hello Arcada Thermometer");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  display_init();
  measure_init();

  arcada.timerCallback(1 /* Hz */, timercallback);
  arcada.display->fillScreen(ARCADA_BLACK);
  measureVoltage();
}

//---- beepIfNeeded ----
void beepIfNeeded(int count) {
  if (countdown < 0 && tempChange != 0.0) {
    // start the counter to hide the tempChange mark later
    countdown = intervals[time_interval];
  } else if (count  == 0 && countdown == 0) {
    // silently hide both the tempChange mark and the counter
    tempChange = 0.0;
    countdown = -1;
  } else if (count != 0 && countdown <= 0) {
    bool goodChange = count > 0 && tempChange < 0.0 || count < 0 && tempChange > 0.0;
    if (sound && ! goodChange) {
      // beep
      arcada.enableSpeaker(true);
      play_tune(audio, sizeof(audio));
      arcada.enableSpeaker(false);
    }
    countdown = intervals[time_interval];
    tempChange = 0.0;
  }
}

//---- loop ----
void loop() {
  bool update_data = false;
  if (measure) {
    measureTemperature();
    measureVoltage();
    measureLight();
    measure = false;
    update_data = true;
  }
  update_data |= processInput();
  if (update_data) {
    updateDisplay();
    int count = updatePixels();
    beepIfNeeded(count);
    analogWrite(13, 0); // LED OFF
  }
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

/*****************************************************************/
