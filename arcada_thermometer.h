#ifndef ARCADA_THERMOMETER
#define ARCADA_THERMOMETER

//---- arcada ----
extern Adafruit_Arcada arcada; // cp437

//---- measure and display ----
extern AverageTemp avgTemp;
extern AverageTemp voltage;
extern uint16_t light;

//---- config ----
#define BR_SIZE 5
#define LBR_SIZE 6
#define INTERVALS_SIZE 7
extern int intervals[];

//---- settings ----
extern int t_set; // in half degrees Celsius
extern bool sound;
extern bool lcd_auto;
extern int time_interval;
extern int lcd_brightness;
extern bool lcd_auto;
extern int led_brightness;
extern bool led_auto;

//---- timer ----
extern volatile int countdown;

//---- függvények ----
extern void measure_init();
extern void measureTemperature();
extern void printTemperature();
extern bool processInput();

#endif
