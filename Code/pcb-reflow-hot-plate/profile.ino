/**
 * PCB Reflow Hot Plate
 * profile.ino 
 * Contains reflow profile related functions
 */

/**
 * @brief Get the target temperature from the reflow profile at given time
 * Finds the two time/temp point the given time lies in between of
 * Interpolates between the two points to get the temperature value at given time
 * @param profile reflow profile
 * @param time time in seconds
 * @return int target temperature
 */
int getReflowTemp(reflow_profile_t profile, int time){
  static int time_1 = 0;
  static int time_2 = 0;
  static int temp_1 = 0;
  static int temp_2 = 0;
  static int target_temp = 0;

  // Find time interval to interpolate between
  for(int i = 0; i < PROFILE_MAX_POINTS; i++){
    if(time >= profile.time_steps[i] && time < profile.time_steps[i+1]){
      time_1 = profile.time_steps[i];
      temp_1 = profile.temp_steps[i];
      time_2 = profile.time_steps[i+1];
      temp_2 = profile.temp_steps[i+1];
      break;
    } else {
      time_1 = 0;
      time_2 = 0;
      temp_1 = 0;
      temp_2 = 0;
    }
  }

  target_temp = map(time, time_1, time_2, temp_1, temp_2);

  return target_temp;

}

/**
 * @brief Get duration of reflow profile from selected profile
 * Searches for the maximum time step defined in the profile
 * @return int maximum time in seconds
 */
int getReflowMaxTime(){
  int max_time = 0;

  for(int i = 0; i < PROFILE_MAX_POINTS; i++){
    // Find the maximum time step in reflow profile
    if(reflow_profile_array[selected_profile].time_steps[i] > max_time){
      max_time = reflow_profile_array[selected_profile].time_steps[i];
    }
  }

  return max_time;
}