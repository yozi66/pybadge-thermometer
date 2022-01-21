#ifndef _AverageTemp_H_
#define _AverageTemp_H_

#ifndef DEVICE_DISCONNECTED_C
#define DEVICE_DISCONNECTED_C -127
#endif

/*
 * Compute two average temperature values: 
 * - exponential moving average
 * - one minute average
 */
class AverageTemp {
public:
  int old_wt = 7; // weight of the old value in exponential moving average
  float hysteresis = 0.05; // display hysteresis to prevent frequent changes in display

  float temp_curr = DEVICE_DISCONNECTED_C; // current temperature - last measured value
  float temp_avg = DEVICE_DISCONNECTED_C; // exponential moving average
  float temp_disp = DEVICE_DISCONNECTED_C; // last display value
  bool temp_inc; // last change direction for display hysteresis
  
  void setTemp(float temp);
};

#endif // _AverageTemp_H_
