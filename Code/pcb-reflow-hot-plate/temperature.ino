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
 */
float readTempSensor(){
  static float read_array[TEMP_FILTER_WINDOW];
  static float sum = 0.0;
  static float read_value = 0.0;
  static int ind = 0;
  static float temp = 0.0;
  static unsigned long last_temp_update = 0;
  static unsigned long cur_time = 0;
  static unsigned long delta_time_ms = 0;

  cur_time = millis();
  delta_time_ms = cur_time - last_temp_update;

  // Reset calculation if called after long
  if(delta_time_ms > (2 * TEMP_UPDATE_DELAY)){
    debugprintln("temp filter reset");

    // Get instantanous temperature read
    if(!thermoCouple.read()){
      read_value = thermoCouple.getTemperature();
    }

    // Prepopulate the moving filter array for instant ramp
    sum = 0;
    for(int i = 0; i < TEMP_FILTER_WINDOW; i++){
      read_array[i] = read_value; // populate the array
      sum += read_value; // add up the sum
    }
    ind = 0;
    temp = read_value;
    last_temp_update = cur_time;

  // Get new temperature value every TEMP_UPDATE_DELAY
  } else if(delta_time_ms > TEMP_UPDATE_DELAY){

    // Get temperature value
    if(!thermoCouple.read()){
      read_value = thermoCouple.getTemperature();
    }

    // Moving averaging filter calculation
    sum = sum - read_array[ind];
    read_array[ind] = read_value;
    sum = sum + read_value;
    ind = (ind + 1) % TEMP_FILTER_WINDOW;
    temp = sum / TEMP_FILTER_WINDOW;
    temp = roundf(temp * 10) / 10;

    last_temp_update = cur_time;

    // debugprint(temp);
    // debugprint(",");
    // debugprintln(read_value);

  }

  return temp;
}