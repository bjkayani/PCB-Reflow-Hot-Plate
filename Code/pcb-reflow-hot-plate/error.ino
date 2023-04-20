/**
 * PCB Reflow Hot Plate
 * error.ino 
 * Contains functions for error checking
 * 
 * A modified circular buffer is implemented to store the temperature and PWM values
 * of a specified time duration. This is then used to check for proper heat up of the
 * hot plate. Slow or no heat up could be due to a heater malfunction or temperature
 * sensing malfunction.
 */


/**
 * @brief Check for errors in operation of the hotplate
 * This function needs to be called in a loop
 * It runs on its own internal timer every ERROR_CHECK_INTERVAL
 * @param set_temp The current target temperature for the hot plate
 * @param active The state of the current working mode
 */
void errorCheckLoop(int set_temp, bool active){
  static unsigned long last_time = 0;
  static unsigned long cur_time = 0;
  static unsigned long delta_time_ms = 0;
  static float temp_rise = 0.0;

  cur_time = millis();
  delta_time_ms = cur_time - last_time;

  // Reset the temp/pwm buffer if heat/reflow is turned off
  if(!active){
    resetBuffer();
  }

  if(delta_time_ms > ERROR_CHECK_INTERVAL){

    tempPwmBufferAdd(cur_temp, heater_pwm); // add temperature and pwm value to buffer

    /**
     * Check for heater malfunction
     * If PWM was pegged at 100% but the rise in temperature was lower than the predefined value,
     * there is probably an issue with the heater or temperature sensor
     * 
     * getPwmSum() >= (BUFFER_SIZE*PWM_MAX) checks if PWM was set to 100% for the entire buffer time
     * cur_temp < HEAT_UP_CHECK_TEMP_LIMIT limits the checking range since heating slows down at high temps
     */
    if(getPwmSum() >= (BUFFER_SIZE*PWM_MAX) && !heat_up_error && cur_temp < HEAT_UP_CHECK_TEMP_LIMIT){
      temp_rise = tempBufferGetLatest() - tempBufferGetOldest();
      if(temp_rise < HEAT_UP_MIN_TEMP_RISE){
        heat_up_error = true;
        debugprintln("heat up error");
      }
    }

    // Check for over temperature
    if(cur_temp > HIGHTEMP_ERROR_LIMIT && !high_temp_error){
      high_temp_error = true;
      debugprintln("hightemp error");
    }

    last_time = cur_time;
  }

}

// ---------- Buffer Implementation ----------   

/**
 * @brief Add temperature and PWM value to buffer
 * @param temp Current temperature (float) 
 * @param pwm Current commanded PWM dutycycle
 */
void tempPwmBufferAdd(float temp, int pwm) {
    temp_buffer[buffer_head] = temp;
    pwm_buffer[buffer_head] = pwm;
    buffer_head = (buffer_head + 1) % BUFFER_SIZE;
    if (buffer_count == BUFFER_SIZE) {
        buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    } else {
        buffer_count++;
    }
}

/**
 * @brief Get the latest temperature value in buffer
 * @return float 
 */
float tempBufferGetLatest() {
    int latest_index = (buffer_head - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    return temp_buffer[latest_index];
}

/**
 * @brief Get the oldest temperature value in buffer
 * @return float 
 */
float tempBufferGetOldest() {
    return temp_buffer[buffer_tail];
}


/**
 * @brief Get the latest PWM value in buffer
 * @return int 
 */
int pwmBufferGetLatest() {
    int latest_index = (buffer_head - 1 + BUFFER_SIZE) % BUFFER_SIZE;
    return pwm_buffer[latest_index];
}

/**
 * @brief Get the oldest PWM value in buffer
 * @return int 
 */
int pwmBufferGetOldest() {
    return pwm_buffer[buffer_tail];
}

/**
 * @brief Get the sum of all PWM values in the buffer
 * @return unsigned long 
 */
unsigned long getPwmSum(){
  unsigned long sum = 0;
  for (int i = 0; i < buffer_count; i++) {
      int index = (buffer_tail + i) % BUFFER_SIZE;
      sum += pwm_buffer[index];
  }
  return sum;
}

/**
 * @brief Reset the buffer
 */
void resetBuffer(){
  buffer_head = 0;
  buffer_tail = 0;
  buffer_count = 0;
}
