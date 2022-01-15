
#include <Adafruit_Arcada.h>
#include <Adafruit_SPIFlash.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "audio.h"
#include "AverageTemp.h"

//---- arcada ----
Adafruit_Arcada arcada;
extern Adafruit_SPIFlash Arcada_QSPI_Flash;
uint32_t buttons, last_buttons;
uint8_t j = 0;  // neopixel counter for rainbow

//---- brightness ----
#define BR_SIZE 12
int brightness_table[BR_SIZE] = {1, 2, 3, 5, 9, 14, 22, 36, 59, 95, 155, 250};

//---- DS18B20 ----
#define ONE_WIRE_BUS 3
#define TEMPERATURE_PRECISION 12
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);
DeviceAddress oneWire_addr;

//---- colors ----

uint32_t PX_RED = arcada.pixels.Color(1,0,0);
uint32_t PX_YELLOW = arcada.pixels.Color(1,1,0);
uint32_t PX_GREEN = arcada.pixels.Color(0,1,0);
uint32_t PX_CYAN = arcada.pixels.Color(0,1,1);
uint32_t PX_BLUE = arcada.pixels.Color(0,0,1);

//---- data to display ----
int countdown = 59;
int t_set = 21;
AverageTemp avgTemp;
int time_interval = 60;
int lcd_brightness = 9;
int led_brightness = 9;

//---- timer callback ----
// Check the timer callback, this function is called every second!
// volatile uint16_t milliseconds = 0;
bool flag = true;
void timercallback() {
  if (flag) {
    analogWrite(13, 1); // weak red light on the LED
  } else {
    analogWrite(13, 0); // LED OFF
  }
  flag = ! flag;
}

//---- printTemperature ----
void printTemperature() {
  avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  arcada.display->fillRect(0, 56, 128, 40, ARCADA_BLACK);
  arcada.display->setCursor(12, 56);
  arcada.display->setTextSize(4);
  arcada.display->print(avgTemp.temp_disp, 1);
  arcada.display->setTextSize(1);
  arcada.display->print("\xF8" "C");
  ds18b20.requestTemperatures();
}

void setup() {
  //while (!Serial);

  Serial.println("Hello! Arcada PyGamer test");
  if (!arcada.arcadaBegin()) {
    Serial.print("Failed to begin");
    while (1);
  }
  arcada.displayBegin();
  Serial.println("Arcada display begin");

  for (int i=0; i<BR_SIZE; i++) {
    arcada.setBacklight(brightness_table[i]);
    delay(100);
  }
  arcada.display->fillScreen(ARCADA_BLACK);

  arcada.display->setRotation(2);
  arcada.display->setCursor(0, 0);
  arcada.display->setTextWrap(true);

  buttons = last_buttons = 0;

  ds18b20.begin();
  int deviceCount = ds18b20.getDeviceCount();
  arcada.display->print("Found ");
  arcada.display->print(deviceCount);
  arcada.display->println(" DS18B20");
  if (deviceCount > 0) {
    ds18b20.getAddress(oneWire_addr, 0);
    ds18b20.requestTemperatures();
    delay(750);
    avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  }
  printTemperature();
  delay(1500);
  arcada.timerCallback(1 /* Hz */, timercallback);
  arcada.display->fillScreen(ARCADA_BLACK);
}

void loop() {
  delay(100);  // add some delay so our screen doesnt flicker
  bool playsound = false;

  // first line
  arcada.display->fillRect(0, 0, 128, 16, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->setCursor(0, 4);
  arcada.display->printf("%3d", countdown);
  arcada.display->print("s   ");
  arcada.display->setCursor(52, 4);
  arcada.display->print(t_set);
  arcada.display->print("\xF8" "C ");
  // Read battery
  arcada.display->setCursor(96, 4);
  float vbat = arcada.readBatterySensor();
  arcada.display->print(vbat); arcada.display->println("V");

  // average temperature
  printTemperature();

  // darkgreen row
  arcada.display->fillRect(0, 126, 128, 10, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_DARKGREEN);
  // Read light sensor
  arcada.display->setCursor(0, 126);
  arcada.display->printf("L%d   ", arcada.readLightSensor());
  // current temperature
  arcada.display->setCursor(96, 126);
  arcada.display->print(avgTemp.temp_curr, 2);

  // last row
  arcada.display->fillRect(0, 148, 128, 10, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_GREEN);
  arcada.display->setCursor(0, 148);
  arcada.display->printf("%3ds", time_interval);
  arcada.display->setCursor(40, 148);
  arcada.display->printf("lcd:%2d ", lcd_brightness);
  arcada.display->setCursor(90, 148);
  arcada.display->printf("led:%2d ", led_brightness);

  // LEDs
  arcada.pixels.setPixelColor(0, PX_BLUE * brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(1, PX_CYAN * brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(2, PX_GREEN * brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(3, PX_YELLOW * brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(4, PX_RED * brightness_table[led_brightness]);
  arcada.pixels.show();

  /*
  uint8_t pressed_buttons = arcada.readButtons();
  arcada.display->fillRect(0, 70, 160, 60, ARCADA_BLACK);
  
  if (pressed_buttons & ARCADA_BUTTONMASK_A) {
    Serial.print("A");
    arcada.display->drawCircle(145, 100, 10, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_B) {
    Serial.print("B");
    arcada.display->drawCircle(120, 100, 10, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_SELECT) {
    Serial.print("Sel");
    arcada.display->drawRoundRect(60, 100, 20, 10, 5, ARCADA_WHITE);
  }
  if (pressed_buttons & ARCADA_BUTTONMASK_START) {
    Serial.print("Sta");
    arcada.display->drawRoundRect(85, 100, 20, 10, 5, ARCADA_WHITE);
  }

  arcada.display->drawCircle(20, 100, 20, ARCADA_WHITE);
  int joyX = arcada.readJoystickX() / 512.0 * 15.0 + 20; 
  int joyY = arcada.readJoystickY() / 512.0 * 15.0 + 100; 
  arcada.display->fillCircle(joyX, joyY, 5, ARCADA_WHITE);
  Serial.println();
  */

  last_buttons = buttons;

  if (playsound) {
    arcada.enableSpeaker(true);
    play_tune(audio, sizeof(audio));
    arcada.enableSpeaker(false);
  }
}

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
