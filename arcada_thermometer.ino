
#include <Adafruit_Arcada.h>
#include "audio.h"
#include "AverageTemp.h"

#include "arcada_thermometer.h"

/***** TODO *****
 * arcada_thermometer.ino
 * arcada_thermometer.h
 * measure.cpp
 * decide.cpp
 * action.cpp
 */

// DEBUG flag -> wait for Serial
#define DEBUG 0

//---- arcada ----
Adafruit_Arcada arcada; // cp437

//---- data tables and constants ---
int brightness_table[BR_SIZE] = {1,  4,  16,  64,   255};
uint16_t lcd_low[BR_SIZE] =     {0,  2,  10,  70,   500}; // thresholds to switch to lower lcd brightness
uint16_t lcd_high[BR_SIZE] =    {2, 20, 140, 750, 65535}; // thresholds to switch to higher lcd brightness

uint16_t led_low[LBR_SIZE] =         {0,  0,  4,  50,  200,   800}; // thresholds to switch to lower led brightness
uint16_t led_high[LBR_SIZE] =        {0, 10, 50, 500,  900, 65535}; // thresholds to switch to higher led brightness

int intervals[INTERVALS_SIZE] = {5, 15, 30, 60, 120, 180, 300};
char speaker[] = { 0x0e, 0x00 };

#define VOLTAGES_SIZE 6
// display state ->            RED    0    1    2    3    4   +
float voltages[VOLTAGES_SIZE] = { 3.45, 3.6, 3.75, 3.9, 4.04, 4.18 };

//---- data to display ----
int t_set = 43; // in half degrees Celsius
AverageTemp avgTemp;
#define BUFSIZE 12
char tempStr[BUFSIZE];
float oldTemp;
char oldLastDigit;
float tempChange;
#define UP_ARROW   0x18
#define DOWN_ARROW 0x19

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

//---- measureVoltage ----
void measureVoltage() {
  voltage.setTemp(arcada.readBatterySensor());
}

//---- measureLight ----
void measureLight() {
    light = arcada.readLightSensor();
    if (lcd_auto) {
      if (light > lcd_high[lcd_brightness]) {
        lcd_brightness++;
      } else if (light < lcd_low[lcd_brightness]) {
        lcd_brightness--;
      }
    }
    if (led_auto) {
      if (light > led_high[led_brightness]) {
        led_brightness++;
      } else if (light < led_low[led_brightness]) {
        led_brightness--;
      }
    }
}

//---- printArrow ----
void printArrow(char arrow) {
    arcada.display->setCursor(108, 60);
    arcada.display->setTextSize(2);
    arcada.display->printf("%c", arrow);
}

//---- printTemperature ----
// side effect: set tempChange on visible change
void printTemperature() {
  snprintf(tempStr, BUFSIZE, "%4.1f", avgTemp.temp_disp);
  bool visibleChange = tempStr[3] != oldLastDigit;
  oldLastDigit = tempStr[3];
  arcada.display->setCursor(12, 48);
  arcada.display->setTextSize(4);
  arcada.display->print(tempStr);
  if (visibleChange) {
    tempChange = avgTemp.temp_disp - oldTemp;
    oldTemp = avgTemp.temp_disp;
    printArrow(tempChange > 0 ? UP_ARROW : DOWN_ARROW);
  } else if (tempChange == 0.0) {
    printArrow(' '); // clear the arrow
  }
  arcada.display->setCursor(108, 48);
  arcada.display->setTextSize(1);
  arcada.display->print("\xF8" "C");
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
  arcada.displayBegin();
  Serial.println("Arcada display begin");

  for (int i=0; i<BR_SIZE; i++) {
    arcada.setBacklight(brightness_table[i]);
    delay(10);
  }
  arcada.display->fillScreen(ARCADA_BLACK);

  arcada.display->setRotation(2);
  arcada.display->setCursor(0, 0);
  arcada.display->setTextWrap(true);
  arcada.display->cp437();

  measure_init();

  arcada.timerCallback(1 /* Hz */, timercallback);
  arcada.display->fillScreen(ARCADA_BLACK);
  voltage.hysteresis = 0.005;
  voltage.old_wt = 7;
  measureVoltage();
}

//---- drawBattery ----
void drawBattery(float vbat) {
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(96, 14);
  uint16_t color = vbat < voltages[0] ? ARCADA_RED : ARCADA_GREEN;
  arcada.display->setTextColor(color, ARCADA_BLACK);
  arcada.display->print(vbat); arcada.display->println("V");
  float percentage = (vbat - 3.31) / 0.88 * 100;
  percentage = max(0.0, percentage);
  percentage = min(100.0, percentage);
  arcada.display->setCursor(54, 4);
  arcada.display->printf("%3.0f%%", percentage);
  #define BAT_LEFT 98
  #define BAT_TOP 3
  #define BAT_HEIGHT 9
  #define BAT_UNITS (VOLTAGES_SIZE - 2)
  #define UNIT_WIDTH 3
  #define UNIT_SPACE 2
  arcada.display->drawRect(BAT_LEFT + 2, BAT_TOP, BAT_UNITS * UNIT_WIDTH + (BAT_UNITS + 1) * UNIT_SPACE + 2, 
      BAT_HEIGHT, color);
  arcada.display->drawRect(BAT_LEFT, BAT_TOP + 3, 3, BAT_HEIGHT - 6, color);
  uint16_t maxColor = vbat > voltages[VOLTAGES_SIZE - 1] ? ARCADA_GREEN : ARCADA_BLACK;
  arcada.display->drawPixel(BAT_LEFT + 1, BAT_TOP + 4, maxColor);
  arcada.display->drawFastHLine(BAT_LEFT, BAT_TOP + 2, 2, maxColor);
  arcada.display->drawFastHLine(BAT_LEFT, BAT_TOP + 6, 2, maxColor);

  int i;
  for (i = 0; i < BAT_UNITS; i++) {
    int x = BAT_LEFT + 3 + (BAT_UNITS - i - 1) * (UNIT_WIDTH + UNIT_SPACE) + UNIT_SPACE;
    arcada.display->fillRect(x, BAT_TOP + 2, UNIT_WIDTH, BAT_HEIGHT - 4, vbat > voltages[i + 1] ? color : ARCADA_BLACK);
  }
}

//---- updateDisplay ----
void updateDisplay(uint16_t light) {
  arcada.setBacklight(brightness_table[lcd_brightness]);

  // first line
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 4);
  arcada.display->printf("%5.1f" "\xF8" "C", t_set / 2.0);
  drawBattery(voltage.temp_disp);

  // current temp
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(42, 14);
  arcada.display->printf("%7.2f", avgTemp.temp_curr);

  // countdown
  arcada.display->setCursor(48, 96);
  arcada.display->setTextSize(2);
  if (countdown < 0) {
    arcada.display->print("   ");
  } else {
    arcada.display->setTextColor(ARCADA_YELLOW, ARCADA_BLACK);
    arcada.display->printf("%3d", countdown);
  }
  arcada.display->setTextSize(1);

  // light sensor
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(light > 9999 ? 0 : 6, 128);
  arcada.display->printf("%-4d", light);

  // speaker flag
  arcada.display->setTextColor(sound ? ARCADA_GREEN : ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(6, 14);
  arcada.display->print(speaker);

  // last row
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 148);
  arcada.display->printf("%3ds", intervals[time_interval]);
  arcada.display->setCursor(40, 148);
  arcada.display->printf("lcd:%1d%c", lcd_brightness + 1, lcd_auto ? 'A' : ' ');
  arcada.display->setCursor(90, 148);
  arcada.display->printf("led:%1d%c", led_brightness, led_auto ? 'A' : ' ');

  // average temperature
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  printTemperature();
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
    updateDisplay(light);
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
