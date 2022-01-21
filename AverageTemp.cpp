#include "AverageTemp.h"

void AverageTemp::setTemp(float temp) {
  temp_curr = temp;
  if (temp != DEVICE_DISCONNECTED_C) {
    // compute moving average
    if (temp_avg == DEVICE_DISCONNECTED_C) {
      temp_avg = temp;
    } else {
      temp_avg = (old_wt * temp_avg + temp) / (old_wt + 1);
    }
    // hysteresis to prevent frequent changes due to measurement noise
    if (temp_disp == DEVICE_DISCONNECTED_C) {
      temp_disp = temp;
    } else {
      bool update = false;
      if (temp_inc) {
        update = temp_avg > temp_disp;
        if (!update && temp_avg < temp_disp - hysteresis) {
          update = true;
          temp_inc = false; 
        }
      } else {
        update = temp_avg < temp_disp;
        if (!update && temp_avg > temp_disp + hysteresis) {
          update = true;
          temp_inc = true;
        }
      }
      if (update) {
        temp_disp = temp_avg;
      }
    }
  }
}
