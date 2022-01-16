
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
uint32_t PX_BLACK = arcada.pixels.Color(0,0,0);

//---- data to display ----
volatile int countdown = -1;
int t_set = 21;
AverageTemp avgTemp;
int time_interval = 3;
int lcd_brightness = 2;
int led_brightness = 3;

#define BR_SIZE 5
int brightness_table[BR_SIZE] = {1, 4, 16, 63, 250};
#define LBR_SIZE 6
int led_brightness_table[LBR_SIZE] = {0, 14, 29, 59, 123, 255};

//---- timing ---
#define INTERVALS_SIZE 7
int intervals[INTERVALS_SIZE] = {5, 15, 30, 60, 120, 180, 300};

//---- timer callback ----
// Check the timer callback, this function is called every second!
// volatile uint16_t milliseconds = 0;
volatile bool flag = true;
void timercallback() {
  analogWrite(13, 0); // LED OFF
  if (countdown > 0) {
    countdown--;
  }
  flag = false;
}

//---- printTemperature ----
void printTemperature() {
  avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  arcada.display->setCursor(12, 48);
  arcada.display->setTextSize(4);
  arcada.display->printf("%4.1f", avgTemp.temp_disp);
  arcada.display->setTextSize(1);
  arcada.display->print("\xF8" "C");
  ds18b20.requestTemperatures();
}

void setup() {
  // while (!Serial);
  Serial.begin(115200);

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
    if (intervals[time_interval] < countdown) {
      countdown = intervals[time_interval];
    }
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
  if (y > JOY_THRESHOLD && led_brightness < LBR_SIZE - 1) {
    led_brightness++;
  }
  if (y < -JOY_THRESHOLD && led_brightness > 0) {
    led_brightness--;
  }
  last_buttons = buttons;
}

void loop() {
  // delay(100);  // add some delay so our screen doesnt flicker
  while(flag);
  flag = true;
  bool playsound = false;
  int x = arcada.readJoystickX();
  int y = arcada.readJoystickY();

  processInput(x, y);
  arcada.setBacklight(brightness_table[lcd_brightness]);

  // first line
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 4);
  arcada.display->printf("%3d" "\xF8" "C", t_set);
  // current temp
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(36, 4);
  arcada.display->printf("%7.2f", avgTemp.temp_curr);
  // Read battery
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(96, 4);
  float vbat = arcada.readBatterySensor();
  arcada.display->print(vbat); arcada.display->println("V");

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

  // lower darkgreen row
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  // Read light sensor
  arcada.display->setCursor(0, 128);
  arcada.display->printf("L%-4d", arcada.readLightSensor());
  // joystick
  arcada.display->setCursor(40, 128);
  arcada.display->printf("x:%-4d", x);
  arcada.display->setCursor(90, 128);
  arcada.display->printf("y:%-4d", y);

  // last row
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 148);
  arcada.display->printf("%3ds", intervals[time_interval]);
  arcada.display->setCursor(40, 148);
  arcada.display->printf("lcd:%2d", lcd_brightness + 1);
  arcada.display->setCursor(90, 148);
  arcada.display->printf("led:%2d", led_brightness);

  // LEDs
  float delta = avgTemp.temp_disp - t_set;
  uint32_t color = (delta > 0 ? PX_RED : PX_BLUE) * led_brightness_table[led_brightness];
  int count = delta / 0.499;
  if (count < 0) {
    count = -count;
  }
  // Serial.printf("delta: %f, count: %d\n", delta, count);
  arcada.pixels.setPixelColor(2, count > 0 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(3, count > 1 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(1, count > 2 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(4, count > 3 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(0, count > 4 ? color : PX_BLACK);
  arcada.pixels.show();

  if (count == 0) {
    countdown = -1;
  } else if (countdown <= 0) {
    countdown = intervals[time_interval];
    playsound = true;
  }

  analogWrite(13, 1); // weak red light on the LED
  if (playsound) {
    arcada.enableSpeaker(true);
    play_tune(audio, sizeof(audio));
    arcada.enableSpeaker(false);
  }

  // average temperature
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
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
