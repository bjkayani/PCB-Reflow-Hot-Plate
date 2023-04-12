/**
 * PCB Reflow Hot Plate
 * buzzer.ino 
 * Contains functions for the buzzer
 * 
 * TODO: Consolidate the functions and improve implementation
 * TODO: Explore other buzzer options 
 */

/**
 * @brief Blocking beep the buzzer for specific duration
 * @param duration_ms duration to beep for
 */
void buzzerBeepBlocking(int duration_ms){
  // Get buzzer enabled value from settings
  int enabled = setting_items[SETTINGS_BUZZER].values[setting_items[SETTINGS_BUZZER].set_value_index];

  if(enabled){
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration_ms);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

/**
 * @brief Non blocking beep the buzzer for specific duration
 * @param duration_ms duration to beep for
 */
void buzzerBeep(int duration_ms){
  // Get buzzer enabled value from settings
  int enabled = setting_items[SETTINGS_BUZZER].values[setting_items[SETTINGS_BUZZER].set_value_index];

  if(enabled){
    current_buzzer_duration = duration_ms;

    if(!buzzer_on){
      digitalWrite(BUZZER_PIN, HIGH);
      buzzer_on = true;
      buzzer_on_time = millis();
    } 
  }
}

/**
 * @brief Buzzer loop function for non blocking operation
 * Call this function in a loop
 */
void buzzerLoop(){
  static unsigned long cur_time = 0;
  int enabled = setting_items[SETTINGS_BUZZER].values[setting_items[SETTINGS_BUZZER].set_value_index];

  if(enabled){
    cur_time = millis();

    if((cur_time - buzzer_on_time) > current_buzzer_duration){
      digitalWrite(BUZZER_PIN, LOW);
      buzzer_on = false;
    }
  }
}