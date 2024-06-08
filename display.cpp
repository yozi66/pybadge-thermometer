#include "Arduino.h"
#include <Adafruit_Arcada.h>
#include "AverageTemp.h"
#include "arcada_thermometer.h"

//---- menu ----
#define MENU_OFF -1
#define MENU_SAVE 0
#define MENU_PROFILE 1
#define MENU_LED 2
#define MENU_SIZE 3

#define MENU_TIME 10

int menuSelected = MENU_OFF;
int menuCountdown = MENU_TIME;

//---- brightness ----

int brightness_table[BR_SIZE] = {1,  4,  16,  64,   255};

//---- graphic characters ----

#define UP_ARROW   0x18
#define DOWN_ARROW 0x19
char speaker[] = { 0x0e, 0x00 };

//---- buffer and change detection vars ----
#define BUFSIZE 12
char tempStr[BUFSIZE];
float oldTemp;
char oldLastDigit;

//---- voltage level thresholds ----
#define VOLTAGES_SIZE 6
// display state ->            RED      0      1      2      3      4     +
float voltages[VOLTAGES_SIZE] = { 3.374, 3.428, 3.643, 3.858, 4.073, 4.180 };

//---- display_init ----

void display_init() {
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
  const char * format = avgTemp.temp_disp < -10 ? "%4.0f" : "%4.1f";
  snprintf(tempStr, BUFSIZE, format, avgTemp.temp_disp);
  bool visibleChange = tempStr[3] != oldLastDigit;
  oldLastDigit = tempStr[3];
  arcada.display->setCursor(12, 48);
  arcada.display->setTextSize(4);
  arcada.display->print(tempStr);
  if (visibleChange) {
    tempChange = avgTemp.temp_disp - oldTemp;
    oldTemp = avgTemp.temp_disp;
    printArrow(tempChange > 0 ? UP_ARROW : DOWN_ARROW);
    changeCountdown = intervals[atConfig.time_interval]; // (re)start the change counter to hide the tempChange mark later
  } else if (tempChange == 0.0) {
    printArrow(' '); // clear the arrow
    changeCountdown = -1;
  }
  arcada.display->setCursor(108, 48);
  arcada.display->setTextSize(1);
  arcada.display->print("\xF8" "C");
}

//---- drawBattery ----
void drawBattery(float vbat) {
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(96, 14);
  uint16_t color = vbat < voltages[0] ? ARCADA_RED : ARCADA_GREEN;
  arcada.display->setTextColor(color, ARCADA_BLACK);
  arcada.display->print(vbat); arcada.display->println("V");
  float percentage = (vbat - 3.32) / 0.86 * 100;
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
void updateDisplay() {
  int brite = atConfig.lcd_brightness;
  if (atConfig.lcd_auto && atConfig.dim && brite > 0) {
    brite--;
  }
  arcada.setBacklight(brightness_table[brite]);

  // first line
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 4);
  arcada.display->printf("%5.1f" "\xF8" "C", atConfig.t_set / 2.0);
  drawBattery(voltage.temp_disp);

  // speaker flag
  arcada.display->setTextColor(atConfig.sound ? ARCADA_GREEN : ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(6, 14);
  arcada.display->print(speaker);

  // current temp
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(42, 14);
  arcada.display->printf("%7.2f", avgTemp.temp_curr);

  // bell countdown
  arcada.display->setCursor(48, 96);
  arcada.display->setTextSize(2);
  if (bellCountdown < 0 || ! atConfig.countdown) {
    arcada.display->print("   ");
  } else {
    arcada.display->setTextColor(ARCADA_YELLOW, ARCADA_BLACK);
    arcada.display->printf("%3d", bellCountdown);
  }
  arcada.display->setTextSize(1);

  // light sensor
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(light > 9999 ? 0 : 6, 128);
  arcada.display->printf("%-4d", light);

  // current profile
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(42, 128);
  arcada.display->print(profiles[atConfig.getProfile()]);

  // change countdown
  arcada.display->setCursor(108, 128);
  arcada.display->setTextSize(1);
  if (changeCountdown < 0) {
    arcada.display->print("   ");
  } else {
    arcada.display->printf("%3d", changeCountdown);
  }

  // last row
  arcada.display->setTextColor(atConfig.countdown ? ARCADA_GREEN : ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 148);
  arcada.display->printf("%3ds", intervals[atConfig.time_interval]);
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(40, 148);
  arcada.display->printf("lcd:%1d%c", atConfig.lcd_brightness + (atConfig.dim && atConfig.lcd_auto ? 0 : 1), atConfig.lcd_auto ? atConfig.dim ? 'd' : 'A' : ' ');
  arcada.display->setCursor(90, 148);
  arcada.display->printf("led:%1d%c", atConfig.led_brightness, atConfig.led_auto ? 'A' : ' ');

  // average temperature
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  printTemperature();
}
