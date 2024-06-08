#include "ATConfig.h"

const char *profiles[NUM_PROFILES] = {" Home ", "Winter", "Sommer", " Other"};

int ATConfig::getProfile() {
    return profile;
}
