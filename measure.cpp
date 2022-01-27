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
}

//---- measureTemperature ----
void measureTemperature() {
  avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  ds18b20.requestTemperatures();
}
