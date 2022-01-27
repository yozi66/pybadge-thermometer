#ifndef ARCADA_THERMOMETER
#define ARCADA_THERMOMETER

//---- arcada ----
extern Adafruit_Arcada arcada; // cp437

//---- measure and display ----
extern AverageTemp avgTemp;

extern void measure_init();
extern void measureTemperature();
extern void printTemperature();

#endif
