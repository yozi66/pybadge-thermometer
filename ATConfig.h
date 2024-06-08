#ifndef AT_CONFIG
#define AT_CONFIG

#include <ArduinoJson.h>

#define AT_CONFIG_FILENAME "/at_config.json"

#define NUM_PROFILES 4
extern const char *profiles[NUM_PROFILES];

class ATConfig {
public:
  int t_set = 43; // in half degrees Celsius
  int time_interval = 2;
  int lcd_brightness = 2;
  bool lcd_auto = true;
  bool dim = false;
  int led_brightness = 3;
  bool led_auto = true;
  bool sound = true;
  bool countdown = true;
  int getProfile();
private:
  int profile = 0;
  StaticJsonDocument<256> pbt_configJSON;
};

#endif
