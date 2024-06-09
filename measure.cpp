#include "Arduino.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Arcada.h>
#include "AverageTemp.h"
#include "arcada_thermometer.h"

//---- DS18B20 ----
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DeviceAddress oneWire_addr;

//---- buttons ----
uint32_t buttons;

//---- light levels ----
uint16_t lcd_low[BR_SIZE] =       {0,  0,   3,  60,   600}; // thresholds to switch to lower lcd brightness
uint16_t lcd_high[BR_SIZE] =      {0,  4,  80, 800, 65535}; // thresholds to switch to higher lcd brightness

uint16_t led_low[LBR_SIZE] =  {0,  0, 12,  45, 180,   675}; // thresholds to switch to lower led brightness
uint16_t led_high[LBR_SIZE] = {0, 16, 60, 240, 900, 65535}; // thresholds to switch to higher led brightness

//---- measure_init ----
void measure_init() {
  ds18b20.begin();
  int deviceCount = ds18b20.getDeviceCount();
  arcada.display->print("Found ");
  arcada.display->print(deviceCount);
  arcada.display->println(" DS18B20");
  if (deviceCount > 0) {
    ds18b20.getAddress(oneWire_addr, 0);
    ds18b20.setWaitForConversion(false);
    ds18b20.requestTemperatures();
    delay(750);
    avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
    printTemperature();
    tempChange = 0.0;
    changeCountdown = -1;
  }
  delay(300);
  voltage.hysteresis = 0.005;
  voltage.old_wt = 7;
}

//---- measureVoltage ----
void measureVoltage() {
  voltage.setTemp(arcada.readBatterySensor());
}

//---- measureLight ----
void measureLight() {
    light = arcada.readLightSensor();
    if (atConfig.lcd_auto) {
      if (light > lcd_high[atConfig.lcd_brightness]) {
        atConfig.lcd_brightness++;
      } else if (light < lcd_low[atConfig.lcd_brightness]) {
        atConfig.lcd_brightness--;
      }
    }
    if (atConfig.led_auto) {
      if (light > led_high[atConfig.led_brightness]) {
        atConfig.led_brightness++;
      } else if (light < led_low[atConfig.led_brightness]) {
        atConfig.led_brightness--;
      }
    }
}

//---- measureTemperature ----
void measureTemperature() {
  avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  ds18b20.requestTemperatures();
}

//---- processInput ----
bool justPressed(uint32_t mask) {
  return buttons & mask;
}

void toggle_sound() {
  if (atConfig.sound) {
    atConfig.sound = false;
  } else if (atConfig.countdown) {
    atConfig.countdown = false;
  } else {
    atConfig.sound = true;
    atConfig.countdown = true;
    bellCountdown = intervals[atConfig.time_interval];
  }
}

void time_interval_up() {
  if (atConfig.time_interval < INTERVALS_SIZE - 1 && atConfig.countdown) {
    atConfig.time_interval++;
  } else {
    toggle_sound();
  }
}

void time_interval_down() {
  if (atConfig.time_interval > 0 && atConfig.countdown) {
    atConfig.time_interval--;
    if (intervals[atConfig.time_interval] < bellCountdown) {
      bellCountdown = intervals[atConfig.time_interval];
    }
    if (intervals[atConfig.time_interval] < changeCountdown) {
      changeCountdown = intervals[atConfig.time_interval];
    }
  } else {
    toggle_sound();
  }
}

void lcd_brightness_up() {
  if (atConfig.dim) {
    atConfig.dim = false;
  } else if (atConfig.lcd_brightness < BR_SIZE - 1) {
    atConfig.lcd_brightness++;
    atConfig.lcd_auto = false;
  } else {
    atConfig.lcd_auto = true;
  }
}

void lcd_brightness_down() {
  if (! atConfig.dim) {
    atConfig.dim = true;
  } else if (atConfig.lcd_brightness > 0) {
    atConfig.lcd_brightness--;
    if (atConfig.lcd_auto && atConfig.lcd_brightness > 0) {
      atConfig.lcd_brightness--;
    }
    atConfig.lcd_auto = false;
  } else {
    atConfig.lcd_auto = true;
  }
}

void led_brightness_up() {
  if (atConfig.led_brightness < LBR_SIZE - 1) {
    atConfig.led_brightness++;
    atConfig.led_auto = false;
  } else {
    atConfig.led_auto = true;
  }
}

void led_brightness_down() {
  if (atConfig.led_brightness > 0) {
    atConfig.led_brightness--;
    atConfig.led_auto = false;
  } else {
    atConfig.led_auto = true;
  }
}

bool processInput() {
  buttons = arcada.readButtons();
  if (justPressed(ARCADA_BUTTONMASK_A)) {
    time_interval_up();
  }
  if (justPressed(ARCADA_BUTTONMASK_B)) {
    time_interval_down();
  }
  if (justPressed(ARCADA_BUTTONMASK_START)) {
    atConfig.t_set++;
  }
  if (justPressed(ARCADA_BUTTONMASK_SELECT)) {
    atConfig.t_set--;
  }
  if (justPressed(ARCADA_BUTTONMASK_RIGHT)) { // UP (due to rotated screen)
    lcd_brightness_up();
  }
  if (justPressed(ARCADA_BUTTONMASK_LEFT)) { // DOWN (due to rotated screen)
    lcd_brightness_down();
  }
  if (justPressed(ARCADA_BUTTONMASK_DOWN)) { // RIGHT (due to rotated screen)
    led_brightness_up();
  }
  if (justPressed(ARCADA_BUTTONMASK_UP)) { // LEFT(due to rotated screen)
    led_brightness_down();
  }
  return buttons;
}
