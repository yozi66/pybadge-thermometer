
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

//---- data tables and constants ---
#define BR_SIZE 5
int brightness_table[BR_SIZE] = {1, 4, 16, 63, 250};
#define LBR_SIZE 6
int led_brightness_table[LBR_SIZE] = {0, 14, 29, 59, 123, 255};
#define INTERVALS_SIZE 7
int intervals[INTERVALS_SIZE] = {5, 15, 30, 60, 120, 180, 300};
char speaker[] = { 0x0e, 0x00 };
#define VOLTAGES_SIZE 6
//                             RED    0    1    2    3    4   +
float voltages[VOLTAGES_SIZE] = { 3.2, 3.4, 3.6, 3.8, 4.0, 4.18 };

//---- data to display ----
int t_set = 43; // in half degrees Celsius
AverageTemp avgTemp;
int time_interval = 3;
int lcd_brightness = 2;
int led_brightness = 3;
bool sound = true;
volatile int countdown = intervals[time_interval];

//---- timer callback ----
// Check the timer callback, this function is called every second!
// volatile uint16_t milliseconds = 0;
volatile bool measure = true;
void timercallback() {
  if (countdown > 0) {
    countdown--;
  }
  analogWrite(13, 1); // weak red light on the LED
  measure = true;
}

//---- measureTemperature ----
void measureTemperature() {
  avgTemp.setTemp(ds18b20.getTempC(oneWire_addr));
  ds18b20.requestTemperatures();
}

//---- printTemperature ----
void printTemperature() {
  arcada.display->setCursor(12, 48);
  arcada.display->setTextSize(4);
  arcada.display->printf("%4.1f", avgTemp.temp_disp);
  arcada.display->setTextSize(1);
  arcada.display->print("\xF8" "C");
}

//---- setup ----
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
  arcada.display->cp437();

  buttons = last_buttons = 0;

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
  arcada.timerCallback(1 /* Hz */, timercallback);
  arcada.display->fillScreen(ARCADA_BLACK);
}

//---- processInput ----
bool justPressed(uint32_t mask) {
  return (buttons & mask) && ! (last_buttons & mask);
}

bool processInput(int x, int y) {
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
  if (justPressed(ARCADA_BUTTONMASK_RIGHT) && lcd_brightness < BR_SIZE - 1) {
    lcd_brightness++;
  }
  if (justPressed(ARCADA_BUTTONMASK_LEFT) && lcd_brightness > 0) {
    lcd_brightness--;
  }
  if (justPressed(ARCADA_BUTTONMASK_DOWN) && led_brightness < LBR_SIZE - 1) {
    led_brightness++;
  }
  if (justPressed(ARCADA_BUTTONMASK_UP) && led_brightness > 0) {
    led_brightness--;
  }
  last_buttons = buttons;
  return buttons;
}

//---- drawBattery ----
void drawBattery(float vbat) {
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(96, 14);
  uint16_t color = vbat < voltages[0] ? ARCADA_RED : ARCADA_GREEN;
  arcada.display->setTextColor(color, ARCADA_BLACK);
  arcada.display->print(vbat); arcada.display->println("V");
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
void updateDisplay(int x, int y) {
  arcada.setBacklight(brightness_table[lcd_brightness]);

  // first line
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  arcada.display->setCursor(0, 4);
  arcada.display->printf("%5.1f" "\xF8" "C", t_set / 2.0);
  // current temp
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  arcada.display->setCursor(42, 4);
  arcada.display->printf("%7.2f", avgTemp.temp_curr);

  // Read battery
  float vbat = arcada.readBatterySensor();
  drawBattery(vbat);

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

  // upper darkgreen row
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
  // Read light sensor
  arcada.display->setCursor(6, 111);
  arcada.display->printf("L%-4d", arcada.readLightSensor());

  // speaker flag
  if (sound) {
    arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  }
  arcada.display->setCursor(6, 128);
  arcada.display->print(speaker);

  // lower darkgreen row
  // joystick
  arcada.display->setTextColor(ARCADA_DARKGREEN, ARCADA_BLACK);
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

  // average temperature
  arcada.display->setTextColor(ARCADA_GREEN, ARCADA_BLACK);
  printTemperature();
}

//---- updatePixels ----
int updatePixels() {
  float delta = avgTemp.temp_disp - (t_set / 2.0);
  uint32_t color = (delta > 0 ? PX_RED : PX_BLUE) * led_brightness_table[led_brightness];
  int count = delta / 0.5;
  if (count < 0) {
    count = -count;
  }
  arcada.pixels.setPixelColor(2, count > 0 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(3, count > 1 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(1, count > 2 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(4, count > 3 ? color : PX_BLACK);
  arcada.pixels.setPixelColor(0, count > 4 ? color : PX_BLACK);
  arcada.pixels.show();
  return count;
}

//---- beepIfNeeded ----
void beepIfNeeded(int count) {
  if (count == 0) {
    countdown = -1;
  } else if (countdown <= 0) {
    countdown = intervals[time_interval];
    if (sound) {
      // beep
      arcada.enableSpeaker(true);
      play_tune(audio, sizeof(audio));
      arcada.enableSpeaker(false);
    }
  }
}

//---- loop ----
void loop() {
  bool update_data = false;
  if (measure) {
    measureTemperature();
    measure = false;
    update_data = true;
  }
  int x = arcada.readJoystickX();
  int y = arcada.readJoystickY();
  update_data |= processInput(x, y);
  if (update_data) {
    updateDisplay(x, y);
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
