#ifndef ARCADA_THERMOMETER
#define ARCADA_THERMOMETER

#include "ATConfig.h"

//---- arcada ----
extern Adafruit_Arcada arcada; // cp437

//---- measure and display ----
extern AverageTemp avgTemp;
extern AverageTemp voltage;
extern uint16_t light;
extern float tempChange;

//---- config ----
#define BR_SIZE 5
#define LBR_SIZE 6
#define INTERVALS_SIZE 5
extern int intervals[];

//---- settings ----
extern ATConfig atConfig;

//---- timer ----
extern volatile int changeCountdown;
extern volatile int bellCountdown;

//---- measure ----
extern void measure_init();
extern void measureTemperature();
extern void printTemperature();
extern bool processInput();
extern void measureVoltage();
extern void measureLight();

//---- action ----
extern bool goodChange();
extern int updatePixels();
extern void beepIfNeeded(int count);

//---- display ----
extern void display_init();
extern void updateDisplay();

#endif
