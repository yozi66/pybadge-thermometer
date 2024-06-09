#include "ATConfig.h"
#include "arcada_thermometer.h"

const char *profiles[NUM_PROFILES] = {"  Home", "Winter", "Sommer", " Other"};

int ATConfig::getProfile() {
    return profile;
}

void ATConfig::profile_up() {
    if (profile < NUM_PROFILES - 1) {
        profile++;
    }
}

void ATConfig::profile_down() {
    if (profile > 0) {
        profile--;
    }
}

void ATConfig::save() {
    arcada.infoBox("Save is not implemented yet");
    arcada.display->fillScreen(ARCADA_BLACK);
}
