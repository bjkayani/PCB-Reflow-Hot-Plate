
/**
 * PCB Reflow Hot Plate
 * heater.ino 
 * Heater PWM and PID step function
 * 
 * TODO: Add temperature discontinuity sensing
 */



/**
 * @brief Manage heater PWM 
 * This function needs to be run in a fast loop
 * Manual PWM implemented due to super slow requirement of the SSR
 */
void heaterPwmLoop(){
  static unsigned long last_on_time = 0;
  static unsigned long last_off_time = 0;
  static bool heater_pwm_on = false;
  static int on_time = 0;
  static int off_time = 0;

  heater_pwm = constrain(heater_pwm, 0, PWM_MAX);

  // Calculate on time and off time for given duty cycle
  on_time = ( PWM_PERIOD * heater_pwm ) / PWM_MAX;
  off_time = ( PWM_PERIOD * (PWM_MAX - heater_pwm) ) / PWM_MAX;

  if(heat_active || reflow_active){

    if(heater_pwm == PWM_MAX){ // Full on for 100% duty cycle
      digitalWrite(SSR_PIN, HIGH);
      heater_pwm_on = true;
      last_on_time = millis();
    } else if(heater_pwm == 0){ // Full off for 0% duty cycle
      digitalWrite(SSR_PIN, LOW);
      heater_pwm_on = false;
      last_off_time = millis();

    // On period has completed, switch to off period
    } else if(heater_pwm_on && (millis() - last_on_time) > on_time){
      heater_pwm_on = false;
      last_off_time = millis();
      digitalWrite(SSR_PIN, LOW);

    // Off period has completed, switch to on period
    } else if (!heater_pwm_on && (millis() - last_off_time) > off_time){
      heater_pwm_on = true;
      last_on_time = millis();
      digitalWrite(SSR_PIN, HIGH);
    }  
  } else {
    // Turn off if heat and reflow modes are both inactive
    digitalWrite(SSR_PIN, LOW);
  }
}


/**
 * @brief PID Step function for heater PWM
 * This functions needs to be called in a loop
 * It will calculate next PWM value using PID control
 * PID calculation is done at PID_STEP_INTERVAL gaps
 * @param set_temp target temperature
 * @param mode reflow or heat mode
 * @return int PWM value from PID algorithm
 * 
 * TODO: Check the delta_t used for calculation to be accurate
 */
int pidStep(float set_temp, int mode){
  // Time tracking variables
  static unsigned long last_time = 0;
  static unsigned long last_deriv_time = 0;
  static unsigned long cur_time = 0;
  static unsigned long delta_time_ms = 0;
  static unsigned long delta_deriv_time_ms = 0;
  static float delta_time_s = 0.0;
  static float delta_deriv_time_s = 0.0;
  // PID calculation variables
  static float Kp = 0.0;
  static float Ki = 0.0;
  static float Kd = 0.0;
  static float temp_error = 0.0;
  static float prev_error = 0.0;
  static float prop = 0.0;
  static float integ = 0.0;
  static float deriv = 0.0;
  static float pid_output = 0.0;
  // Moving averaging filter variables
  static float pid_output_smooth = 0.0;
  static int pid_output_array[PID_FILTER_SIZE];
  static int ind = 0;
  static float sum = 0.0;

  static int heater_check_last_temp = 0;

  // Update PID parameters from struct based on selected mode
  if(mode == HEAT){
    Kp = pid_items[PID_KP].val;
    Ki = pid_items[PID_KI].val;
    Kd = pid_items[PID_KD].val;
  } else if (mode == REFLOW){
    Kp = pid_items[PID_KP_R].val;
    Ki = pid_items[PID_KI_R].val;
    Kd = pid_items[PID_KD_R].val;
  }
  
  // Calculate the time gap between now and the last step
  cur_time = millis();
  delta_time_ms = cur_time - last_time;

  // Reset the calculation if function called after long delay
  if(delta_time_ms > (2 * PID_STEP_INTERVAL)){
    debugprintln("pid reset");

    // Reset tracking variables
    temp_error = 0;
    prev_error = 0;
    prop = 0;
    integ = 0;
    deriv = 0;
    pid_output = 0;
    
    sum = 0;
    // Prepopulate the moving filter array for instant ramp
    pid_output = constrain((Kp * (set_temp - cur_temp)), 0, PWM_MAX); // use proportional error only
    for(int i = 0; i < PID_FILTER_SIZE; i++){
      pid_output_array[i] = pid_output; // populate the array
      sum += pid_output; // add up the sum
    }
    ind = 0;
    pid_output_smooth = pid_output;
    last_time = cur_time;

  // Run PID calculation every PID_STEP_INTERVAL
  } else if(delta_time_ms > PID_STEP_INTERVAL){

    delta_time_s = (float)delta_time_ms / 1000.0; // time since last cal in seconds (float)

    temp_error = set_temp - cur_temp; // proportional error
    prop = temp_error;

    // Only calculate integral when proportional is within a certain range to avoid windup
    if(pid_output < INTEG_ACTIVE_THRESH && pid_output > 0){
      integ = integ + (temp_error * delta_time_s);
    } else {
      integ = 0;
    }
    
    // Calculate time since last derivative calculation
    delta_deriv_time_ms = cur_time - last_deriv_time;
    delta_deriv_time_s = (float)delta_deriv_time_ms / 1000.0;

    // Calculate derivative every DERIV_CALC_INTERVAL
    if(delta_deriv_time_ms > DERIV_CALC_INTERVAL){
      deriv = (temp_error - prev_error) / delta_deriv_time_s;
      last_deriv_time = cur_time;
      prev_error = temp_error;
    }
    
    // Add up all elements for output
    pid_output = (Kp * prop) + (Ki * integ) + (Kd * deriv * PID_KD_PREGAIN);
    pid_output = constrain(pid_output, 0, PWM_MAX);

    // Moving average filter step calculation
    sum -=  pid_output_array[ind];
    pid_output_array[ind] = (int)pid_output;
    sum += (int)pid_output;
    ind = (ind + 1) % PID_FILTER_SIZE;
    pid_output_smooth = sum / PID_FILTER_SIZE;

    last_time = cur_time;
    
    debugprint(set_temp);
    debugprint(",");
    debugprint(cur_temp);
    debugprint(",");
    // debugprint(pid_output);
    // debugprint(",");
    debugprint(pid_output_smooth);
    // debugprint(",");
    // debugprint(prop);
    // debugprint(",");
    // debugprint(integ);
    // debugprint(",");
    // debugprintln(deriv);
    debugprint("\n");
  }

  pid_output_smooth = constrain(pid_output_smooth, 0, PWM_MAX);
  pid_output_smooth = round(pid_output_smooth);

  return pid_output_smooth;
}

