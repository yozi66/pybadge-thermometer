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
  }
  delay(300);
  buttons = last_buttons = 0;
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

bool processInput() {
  buttons = arcada.readButtons();
  // Serial.printf("buttons: %x, ", buttons)
  if (justPressed(ARCADA_BUTTONMASK_A)) {
    if (time_interval < INTERVALS_SIZE - 1) {
      time_interval++;
    } else {
      sound = ! sound;
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_B)) {
    if (time_interval > 0) {
      time_interval--;
      if (intervals[time_interval] < countdown) {
        countdown = intervals[time_interval];
      }
    } else {
      sound = ! sound;
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_START)) {
    t_set++;
  }
  if (justPressed(ARCADA_BUTTONMASK_SELECT)) {
    t_set--;
  }
  if (justPressed(ARCADA_BUTTONMASK_RIGHT)) {
    if (lcd_brightness < BR_SIZE - 1) {
      lcd_brightness++;
      lcd_auto = false;
    } else {
      lcd_auto = true;
    }
  }
  if (justPressed(ARCADA_BUTTONMASK_LEFT)) {
    if (lcd_brightness > 0) {
      lcd_brightness--;
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
