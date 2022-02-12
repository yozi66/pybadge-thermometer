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
uint32_t buttons, last_buttons;

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
  buttons = last_buttons = 0;
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

//---- measureTemperature ----
void measureTemperature() {
  avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  ds18b20.requestTemperatures();
}

//---- processInput ----
bool justPressed(uint32_t mask) {
  return (buttons & mask) && ! (last_buttons & mask);
}

void toggle() {
  if (sound) {
    sound = false;
  } else if (countdown) {
    countdown = false;
  } else {
    sound = true;
    countdown = true;
    bellCountdown = intervals[time_interval];
  }
}

bool processInput() {
  buttons = arcada.readButtons();
  // Serial.printf("buttons: %x, ", buttons)
  if (justPressed(ARCADA_BUTTONMASK_A)) {
    if (time_interval < INTERVALS_SIZE - 1 && countdown) {
      time_interval++;
    } else {
      toggle();
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_B)) {
    if (time_interval > 0 && countdown) {
      time_interval--;
      if (intervals[time_interval] < bellCountdown) {
        bellCountdown = intervals[time_interval];
      }
      if (intervals[time_interval] < changeCountdown) {
        changeCountdown = intervals[time_interval];
      }
    } else {
      toggle();
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_START)) {
    t_set++;
  }
  if (justPressed(ARCADA_BUTTONMASK_SELECT)) {
    t_set--;
  }
  if (justPressed(ARCADA_BUTTONMASK_RIGHT)) {
    if (dim) {
      dim = false;
    } else if (lcd_brightness < BR_SIZE - 1) {
      lcd_brightness++;
      lcd_auto = false;
    } else {
      lcd_auto = true;
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_LEFT)) {
    if (! dim) {
      dim = true;
    } else if (lcd_brightness > 0) {
      lcd_brightness--;
      if (lcd_auto && lcd_brightness > 0) {
        lcd_brightness--;
      }
      lcd_auto = false;
    } else {
      lcd_auto = true;
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_DOWN)) {
    if (led_brightness < LBR_SIZE - 1) {
      led_brightness++;
      led_auto = false;
    } else {
      led_auto = true;
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_UP)) {
    if (led_brightness > 0) {
      led_brightness--;
      led_auto = false;
    } else {
      led_auto = true;
    }
  }
  last_buttons = buttons;
  return buttons;
}
