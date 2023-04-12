/**
 * PCB Reflow Hot Plate
 * parameters.ino 
 * Contains functions to intialize saved parameters and update them
 * Uses the preferences library to save to and retrieve from non-volatile memory
 */

/**
 * @brief Update saved settings 
 */
void updateSettings(){
  preferences.putInt("buzzer", setting_items[SETTINGS_BUZZER].set_value_index);
}

/**
 * @brief Initialize saved settings
 * Initialize or retrieve settings
 */
void initSettings(){
  setting_items[SETTINGS_BUZZER].set_value_index = preferences.getInt("buzzer", 0);
  updateSettings();
}

/**
 * @brief Update saved PID parameter
 * @param index index of parameter to save from pid_items array
 */
void updatePidParams(int index){
    preferences.putFloat(pid_items[index].display_name, pid_items[index].val);
}

/**
 * @brief Initialize saved PID parameters
 * Initialize or retrieve default pid parameters
 */
void initPidParams(){
  // Ensure save key is 14 characters or less
  for(int i=0; i<PID_NUM_ITEMS; i++){
    pid_items[i].val = preferences.getFloat(pid_items[i].display_name, pid_items[i].val);
  }
}

/**
 * @brief Update saved miscellaneous parameters
 */
void updateMiscParams(){
    preferences.putFloat("heat_set_temp", heat_set_temp);
    preferences.putFloat("profile", selected_profile);
}

/**
 * @brief Initialize saved miscellaneous parameters
 * Initialize or retrieve default miscellaneous parameters
 */
void initMiscParams(){
  heat_set_temp = preferences.getFloat("heat_set_temp", 0.0);
  selected_profile = preferences.getFloat("profile", 0);
}

/**
 * @brief Update saved reflow profile
 * 
 * @param index 
 */
void updateReflowProfile(int index){
  static char save_key[10];
  
  // Iterate over all time/temp points in the profile
  for(int i = 0; i < PROFILE_MAX_POINTS; i++){

    // Constrain the time and temp values given defined limits
    if(reflow_profile_array[index].time_steps[i] < 0){
      reflow_profile_array[index].time_steps[i] = 0;
    } else if (reflow_profile_array[index].time_steps[i] > MAX_REFLOW_TIME_S){
      reflow_profile_array[index].time_steps[i] = MAX_REFLOW_TIME_S;
    }

    if(reflow_profile_array[index].temp_steps[i] < MIN_TEMP){
      reflow_profile_array[index].temp_steps[i] = MIN_TEMP;
    } else if (reflow_profile_array[index].temp_steps[i] > MAX_TEMP){
      reflow_profile_array[index].temp_steps[i] = MAX_TEMP;
    }

    // generate save keys and update parameters
    sprintf(save_key, "time%d%d", index, i);
    preferences.putInt(save_key, reflow_profile_array[index].time_steps[i]);
    sprintf(save_key, "temp%d%d", index, i);
    preferences.putInt(save_key, reflow_profile_array[index].temp_steps[i]);
  }

}

/**
 * @brief Initialize saved reflow profile
 */
void initReflowProfiles(){
    static char save_key[10];
    static char name[MAX_DISPLAY_LENGTH];

    // Reset all profiles with default values
    for(int i = 0; i < NUM_PROFILE_MAX; i++){
        for(int j = 0; j < PROFILE_MAX_POINTS; j++){
            reflow_profile_array[i].time_steps[j] = (j * 40);
            reflow_profile_array[i].temp_steps[j] = MIN_TEMP;
        }
        sprintf(name, "Reflow %d", i+1);
        // Set the name for profile
        memcpy(reflow_profile_array[i].display_name, name, sizeof(name));
    }

    // Load in pre-define configurations
    reflow_profile_array [0] = leaded_profile;

  // Initalize empty profiles and save/retreive from memory
    for(int i = 0; i < NUM_PROFILE_MAX; i++){
        for(int j = 0; j < PROFILE_MAX_POINTS; j++){
            // Generate unique save keys
            sprintf(save_key, "time%d%d", i, j);
            reflow_profile_array[i].time_steps[j] = preferences.getInt(save_key, reflow_profile_array[i].time_steps[j]);
            sprintf(save_key, "temp%d%d", i, j);
            reflow_profile_array[i].temp_steps[j] = preferences.getInt(save_key, reflow_profile_array[i].temp_steps[j]);
        }
    }

}