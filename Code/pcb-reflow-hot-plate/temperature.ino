/**
 * PCB Reflow Hot Plate
 * temperature.ino 
 * Contains temperature sensing functions
 */
 

/**
 * @brief Read temperature sensor
 * Reads from the thermocouple MAX6675 sensor
 * Smooths the read value through a moving average filter
 * MAX6675 Datasheet: https://www.analog.com/media/en/technical-documentation/data-sheets/MAX6675.pdf
 * Update rate limited due to the sensing chip limitation
 * @return float smoothed temperature value
 * 
 * TODO: Add sensor unplug detection
 */
float readTempSensor(){
  static float read_array[TEMP_FILTER_WINDOW];
  static float sum = 0.0;
  static float read_value = 0.0;
  static int ind = 0;
  static float temp = 0.0;
  static unsigned long last_time = 0;
  static unsigned long cur_time = 0;
  static unsigned long delta_time_ms = 0;

  cur_time = millis();
  delta_time_ms = cur_time - last_time;

  // Reset calculation if called after long
  if(delta_time_ms > (2 * TEMP_UPDATE_INTERVAL)){
    debugprintln("temp filter reset");

    // Get instantanous temperature read
    if(!thermoCouple.read()){
      read_value = thermoCouple.getTemperature();
    } else {
      debugprintln("temp read error");
      read_value = 999;
    }

    // Prepopulate the moving filter array for instant ramp
    populateMovingFilter(read_value, read_array, TEMP_FILTER_WINDOW, sum);

    ind = 0;
    temp = read_value;
    last_time = cur_time;

  // Get new temperature value every TEMP_UPDATE_INTERVAL
  } else if(delta_time_ms > TEMP_UPDATE_INTERVAL){

    // Get temperature value
    if(!thermoCouple.read()){
      read_value = thermoCouple.getTemperature();
    } else {
      debugprintln("temp read error");
      read_value = 999;
      populateMovingFilter(read_value, read_array, TEMP_FILTER_WINDOW, sum);
    }

    // Moving averaging filter calculation
    sum = sum - read_array[ind];
    read_array[ind] = read_value;
    sum = sum + read_value;
    ind = (ind + 1) % TEMP_FILTER_WINDOW;
    temp = sum / TEMP_FILTER_WINDOW;    
    temp = roundf(temp * 10) / 10;

    last_time = cur_time;

    // debugprint(temp);
    // debugprint(",");
    // debugprintln(read_value);

  }

  return temp;
}

/**
 * @brief Reset moving avg filter array with given value
 * @param value Singular value to populate the filter with
 * @param array Moving avg filter array
 * @param array_size Size of the array
 * @param sum Sum of all values in array (passed by reference)
 */
void populateMovingFilter(float value, float array[], int array_size, float & sum){
    sum = 0;
    for(int i = 0; i < array_size; i++){
      array[i] = value; // populate the array
      sum += value; // add up the sum
    }
}





