#ifndef AT_CONFIG
#define AT_CONFIG

#include <SdFat.h>

#define AT_CONFIG_FILENAME "/at_config.txt"

#define NUM_PROFILES 4
extern const char *profileNames[NUM_PROFILES];

#define ATCONFIG_BUFFER_SIZE 100

class ATSimpleConfig {
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
};

class ATConfig : public ATSimpleConfig {

public:
  int getProfile();
  void profile_up();
  void profile_down();
  void load(const char *filename = AT_CONFIG_FILENAME);
  void save(const char *filename = AT_CONFIG_FILENAME);

private:
  int profile = 0;
  ATSimpleConfig profiles[NUM_PROFILES];

  void createDefaultConfig();
  bool readConfig(const char *filename);
  void readProfile(File file, int index);
  void loadCurrentProfile();
};

#endif
