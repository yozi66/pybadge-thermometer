
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
#define JOY_THRESHOLD 100

//---- brightness ----
#define BR_SIZE 12
int brightness_table[BR_SIZE] = {1, 2, 3, 5, 9, 14, 22, 36, 59, 95, 155, 250};
int led_brightness_table[BR_SIZE] = {0, 14, 19, 25, 33, 45, 60, 80, 107, 143, 191, 255};

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
int countdown = -1;
int t_set = 21;
AverageTemp avgTemp;
int time_interval = 1;
int lcd_brightness = 9;
int led_brightness = 9;

//---- timing ---
#define INTERVALS_SIZE 5
int intervals[INTERVALS_SIZE] = {30, 60, 120, 180, 300};

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
  // arcada.display->fillRect(0, 56, 128, 40, ARCADA_BLACK);
  arcada.display->setCursor(12, 56);
  arcada.display->setTextSize(4);
  arcada.display->printf("%4.1f", avgTemp.temp_disp);
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
    delay(10);
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
  // printTemperature();
  // delay(1500);
  arcada.timerCallback(1 /* Hz */, timercallback);
  arcada.display->fillScreen(ARCADA_BLACK);
}

void processInput(int x, int y) {
  uint32_t buttons = arcada.readButtons();
  if (buttons & ARCADA_BUTTONMASK_A && time_interval < INTERVALS_SIZE - 1) {
    time_interval++;
  }
  if (buttons & ARCADA_BUTTONMASK_B && time_interval > 0) {
    time_interval--;
  }
  if (buttons & ARCADA_BUTTONMASK_START) {
    t_set++;
  }
  if (buttons & ARCADA_BUTTONMASK_SELECT) {
    t_set--;
  }
  if (x > JOY_THRESHOLD && lcd_brightness < BR_SIZE - 1) {
    lcd_brightness++;
  }
  if (x < -JOY_THRESHOLD && lcd_brightness > 0) {
    lcd_brightness--;
  }
  if (y > JOY_THRESHOLD && led_brightness < BR_SIZE - 1) {
    led_brightness++;
  }
  if (y < -JOY_THRESHOLD && led_brightness > 0) {
    led_brightness--;
  }
  last_buttons = buttons;
}

void loop() {
  // delay(100);  // add some delay so our screen doesnt flicker
  bool playsound = false;
  int x = arcada.readJoystickX();
  int y = arcada.readJoystickY();

  processInput(x, y);
  arcada.setBacklight(brightness_table[lcd_brightness]);

  // first line
  // arcada.display->fillRect(0, 0, 128, 16, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 4);
  if (countdown >= 0) {
    arcada.display->printf("%3d", countdown);
    arcada.display->print("s");
  } else {
    arcada.display->print("OK  ");
  }
  arcada.display->setCursor(46, 4);
  arcada.display->printf("%3d" "\xF8" "C", t_set);
  // Read battery
  arcada.display->setCursor(96, 4);
  float vbat = arcada.readBatterySensor();
  arcada.display->print(vbat); arcada.display->println("V");

  // upper darkgreen row
  // arcada.display->fillRect(0, 22, 128, 10, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  // current temperature
  arcada.display->setCursor(40, 22);
  arcada.display->printf("%7.2f", avgTemp.temp_curr);

  // lower darkgreen row
  // arcada.display->fillRect(0, 126, 128, 10, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  // Read light sensor
  arcada.display->setCursor(0, 126);
  arcada.display->printf("L%-4d", arcada.readLightSensor());
  // joystick
  arcada.display->setCursor(40, 126);
  arcada.display->printf("x:%-4d", x);
  arcada.display->setCursor(90, 126);
  arcada.display->printf("y:%-4d", y);

  // last row
  // arcada.display->fillRect(0, 148, 128, 10, ARCADA_BLACK);
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 148);
  arcada.display->printf("%3ds", intervals[time_interval]);
  arcada.display->setCursor(40, 148);
  arcada.display->printf("lcd:%2d", lcd_brightness + 1);
  arcada.display->setCursor(90, 148);
  arcada.display->printf("led:%2d", led_brightness);

  // LEDs
  arcada.pixels.setPixelColor(0, PX_BLUE * led_brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(1, PX_CYAN * led_brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(2, PX_GREEN * led_brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(3, PX_YELLOW * led_brightness_table[led_brightness]);
  arcada.pixels.setPixelColor(4, PX_RED * led_brightness_table[led_brightness]);
  arcada.pixels.show();

  if (playsound) {
    arcada.enableSpeaker(true);
    play_tune(audio, sizeof(audio));
    arcada.enableSpeaker(false);
  }

  // average temperature
  arcada.display->setTextColor(ARCADA_GREEN);
  printTemperature(); // takes 750ms, including measurement

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
