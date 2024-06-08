#include <Adafruit_Arcada.h>
#include "AverageTemp.h"
#include "ATConfig.h"

#include "arcada_thermometer.h"

// DEBUG flag -> wait for Serial
#define DEBUG 0

//---- arcada ----
Adafruit_Arcada arcada; // cp437

//---- data tables and constants ---
int intervals[INTERVALS_SIZE] = {10, 30, 60, 120, 300};

//---- data to display ----
AverageTemp avgTemp;
float tempChange;

//---- settings ----
ATConfig atConfig;

//---- measurements ----
AverageTemp voltage;
uint16_t light = 0;

//---- timer ----
volatile int changeCountdown = -1;
volatile int bellCountdown   = intervals[atConfig.time_interval];

//---- timer callback ----
// This function is called every second
volatile bool measure = true;
void timercallback() {
  if (changeCountdown > 0) {
    changeCountdown--;
  }
  if (bellCountdown > 0) {
    bellCountdown--;
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

/*****************************************************************/
