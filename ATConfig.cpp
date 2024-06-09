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
  File file;
  if (arcada.exists(filename)) {
    Serial.println("file exists, truncating");
    file = arcada.open(filename, O_WRITE | O_TRUNC);
  } else {
    Serial.println("creating config file");
    file = arcada.open(filename, O_WRITE | O_CREAT);
  }
  if (!file) {
    Serial.println(F("Failed to open file to write"));
    arcada.errorBox("Save error");
    arcada.display->fillScreen(ARCADA_BLACK);
    return;
  }
  file.printf("%d\n", profile);
  profiles[profile].t_set = t_set;
  profiles[profile].time_interval = time_interval;
  profiles[profile].lcd_brightness = lcd_brightness;
  profiles[profile].lcd_auto = lcd_auto;
  profiles[profile].dim = dim;
  profiles[profile].led_brightness = led_brightness;
  profiles[profile].led_auto = led_auto;
  profiles[profile].sound = sound;
  profiles[profile].countdown = countdown;
  for (int i = 0; i < NUM_PROFILES; i++) {
    ATSimpleConfig & config = profiles[i];
    file.printf("%d %d %d ", config.t_set, config.time_interval, config.lcd_brightness);
    file.printf("%d %d %d ", config.lcd_auto, config.dim, config.led_brightness);
    file.printf("%d %d %d\n", config.led_auto, config.sound, config.countdown);
    file.printf("t_set time_interval lcd_brightness\n");
    file.printf("lcd_auto dim led_brightness\n");
    file.printf("led_auto sound countdown\n");
  }
  file.flush();
  file.close();
}

void ATConfig::createDefaultConfig() {
    Serial.println("createDefaultConfig...");
    profile = 0;
    // home
    profiles[0].t_set = 40;
    profiles[0].time_interval = 4;
    profiles[0].lcd_brightness = 1;
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
    profiles[2].time_interval = 4;
    profiles[2].lcd_brightness = 2;
    profiles[2].lcd_auto = true;
    profiles[2].dim = false;
    profiles[2].led_brightness = 3;
    profiles[2].led_auto = true;
    profiles[2].sound = true;
    profiles[2].countdown = true;
    // other
    profiles[3].t_set = 55;
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

void ATConfig::readProfile(File & file, ATSimpleConfig & toConfig) {
    Serial.println("Reading profile");
    toConfig.t_set = (int) file.parseInt();
    toConfig.time_interval = (int) file.parseInt();
    toConfig.lcd_brightness = (int) file.parseInt();
    toConfig.lcd_auto = (int) file.parseInt() > 0;
    toConfig.dim = (int) file.parseInt() > 0;
    toConfig.led_brightness = (int) file.parseInt();
    toConfig.led_auto = (int) file.parseInt() > 0;
    toConfig.sound = (int) file.parseInt() > 0;
    toConfig.countdown = (int) file.parseInt() > 0;
    Serial.printf("t_set:%d, time_int:%d, lcd:%d\n", 
      toConfig.t_set, toConfig.time_interval, toConfig.lcd_brightness);
}

bool ATConfig::readConfig(const char * filename) {
  if (! arcada.exists(filename)) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    return false;
  }
  File file = arcada.open(filename);
  if (!file) {
    Serial.println(F("Failed to open file to read"));
    return false;
  }
  profile = (int) file.parseInt();
  for (int i = 0; i < NUM_PROFILES; i++) {
    readProfile(file, profiles[i]);
  }
  uint8_t error = file.getError();
  Serial.printf("Error code: %d\n", error);
  file.close();
  return error == 0;
}

void ATConfig::load(const char *filename) {
  bool success = readConfig(filename);
  Serial.printf("Load success: %d\n", success);
  if (! success) {
    createDefaultConfig();
  }
  Serial.printf("profile: %d\n", profile);
  loadCurrentProfile();
}
