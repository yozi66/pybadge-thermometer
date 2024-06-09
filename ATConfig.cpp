#include "ATConfig.h"
#include "arcada_thermometer.h"
#include <ArduinoJson.hpp>

const char *profileNames[NUM_PROFILES] = {"  home", "winter", "sommer", " other"};

int ATConfig::getProfile() {
    return profile;
}

void ATConfig::loadCurrentProfile() {
    t_set = profiles[profile].t_set;
    time_interval = profiles[profile].time_interval;
    lcd_brightness = profiles[profile].lcd_brightness;
    lcd_auto = profiles[profile].lcd_auto;
    dim = profiles[profile].dim;
    led_brightness = profiles[profile].led_brightness;
    led_auto = profiles[profile].led_auto;
    sound = profiles[profile].sound;
    countdown = profiles[profile].countdown;
}

void ATConfig::profile_up() {
    profile++;
    if (profile >= NUM_PROFILES) {
        profile = 0;
    }
    loadCurrentProfile();
}
void ATConfig::profile_down() {
    profile--;
    if (profile < 0) {
        profile = NUM_PROFILES - 1;
    }
    loadCurrentProfile();
}

void ATConfig::save(const char *filename) {
  File file = arcada.open(filename, O_WRITE);
  if (!file) {
    Serial.println(F("Failed to open file"));
    arcada.errorBox("Save error");
    arcada.display->fillScreen(ARCADA_BLACK);
    return;
  }
  file.printf("%d\n", profile);
  for (int i = 0; i < NUM_PROFILES; i++) {
    ATSimpleConfig & config = profiles[i];
    file.printf("%d %d %d ", config.t_set, config.time_interval, config.lcd_brightness);
    file.printf("%d %d %d ", config.lcd_auto, config.dim, config.led_brightness);
    file.printf("%d %d %d\n", config.led_auto, config.sound, config.countdown);
  }
  file.close();
}

void ATConfig::createDefaultConfig() {
    Serial.println("createDefaultConfig...");
    profile = 0;
    // home
    profiles[0].t_set = 40;
    profiles[0].time_interval = 2;
    profiles[0].lcd_brightness = 2;
    profiles[0].lcd_auto = true;
    profiles[0].dim = true;
    profiles[0].led_brightness = 0;
    profiles[0].led_auto = false;
    profiles[0].sound = false;
    profiles[0].countdown = false;
    // winter
    profiles[1].t_set = 43;
    profiles[1].time_interval = 1;
    profiles[1].lcd_brightness = 2;
    profiles[1].lcd_auto = true;
    profiles[1].dim = false;
    profiles[1].led_brightness = 3;
    profiles[1].led_auto = true;
    profiles[1].sound = true;
    profiles[1].countdown = true;
    // sommer
    profiles[2].t_set = 50;
    profiles[2].time_interval = 2;
    profiles[2].lcd_brightness = 2;
    profiles[2].lcd_auto = true;
    profiles[2].dim = false;
    profiles[2].led_brightness = 3;
    profiles[2].led_auto = true;
    profiles[2].sound = false;
    profiles[2].countdown = true;
    // other
    profiles[3].t_set = 60;
    profiles[3].time_interval = 3;
    profiles[3].lcd_brightness = 2;
    profiles[3].lcd_auto = true;
    profiles[3].dim = false;
    profiles[3].led_brightness = 3;
    profiles[3].led_auto = true;
    profiles[3].sound = false;
    profiles[3].countdown = true;

    Serial.println("Created default config object.");
} 

void ATConfig::readProfile(File file, int i) {
    profiles[i].t_set = (int) file.parseInt();
    profiles[i].time_interval = (int) file.parseInt();
    profiles[i].lcd_brightness = (int) file.parseInt();
    profiles[i].lcd_auto = (int) file.parseInt() > 0;
    profiles[i].dim = (int) file.parseInt() > 0;
    profiles[i].led_brightness = (int) file.parseInt();
    profiles[i].led_auto = (int) file.parseInt() > 0;
    profiles[i].sound = (int) file.parseInt() > 0;
    profiles[i].countdown = (int) file.parseInt() > 0;
}

bool ATConfig::readConfig(const char * filename) {
  File file = arcada.open(filename);
  if (!file) {
    Serial.println(F("Failed to open file"));
    return false;
  }
  profile = (int) file.parseInt();
  for (int i = 0; i < NUM_PROFILES; i++) {
    readProfile(file, i);
  }
  uint8_t error = file.getError();
  file.close();
  return error == 0;
}

void ATConfig::load(const char *filename) {
  bool success = readConfig(filename);
  Serial.printf("Success: %d\n", success);
  if (! success) {
    createDefaultConfig();
  }
  Serial.printf("profile: %d\n", profile);
  t_set = profiles[profile].t_set;
  time_interval = profiles[profile].time_interval;
  lcd_brightness = profiles[profile].lcd_brightness;
  Serial.printf("lcd_auto: %d\n", profiles[profile].lcd_auto);
  lcd_auto = profiles[profile].lcd_auto;
  Serial.printf("lcd_auto: %d\n", lcd_auto);
  dim = profiles[profile].dim;
  led_brightness = profiles[profile].led_brightness;
  led_auto = profiles[profile].led_auto;
  sound = profiles[profile].sound;
  countdown = profiles[profile].countdown;
}
