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
 * @brief Start buzzer sequence
 * @param buzzer_seq The sequence to play
 */
void buzzerSeq(buzzer_sequence_t buzzer_seq){
  cur_buzzer_seq = buzzer_seq;
  buzzer_seq_on_time = millis();
  buzzer_seq_on = true;
}

/**
 * @brief Buzzer loop function for non blocking operation
 * Call this function in a loop
 */
void buzzerLoop(){
  static unsigned long cur_time = 0;
  static unsigned long buzzer_time_delta = 0;
  static unsigned int beep_on_time_min = 0;
  static unsigned int beep_on_time_max = 0;
  static unsigned int beep_off_time_min = 0;
  static unsigned int beep_off_time_max = 0;
  static unsigned int beep_seq_time_max = 0;
  static unsigned int buzzer_seq_index = 1;
  int enabled = setting_items[SETTINGS_BUZZER].values[setting_items[SETTINGS_BUZZER].set_value_index];

  // Play buzzer sequence
  if(buzzer_seq_on){
    if(cur_buzzer_seq.enable_bypass || enabled){
      cur_time = millis();
      buzzer_time_delta = cur_time - buzzer_seq_on_time;
      // Get sequence timings
      beep_on_time_min = (buzzer_seq_index - 1) * (cur_buzzer_seq.buzzer_on_time + cur_buzzer_seq.buzzer_off_time);
      beep_on_time_max = beep_on_time_min + cur_buzzer_seq.buzzer_on_time;
      beep_off_time_min = beep_on_time_max;
      beep_off_time_max = beep_off_time_min + cur_buzzer_seq.buzzer_off_time;
      beep_seq_time_max = (cur_buzzer_seq.buzzer_on_time + cur_buzzer_seq.buzzer_off_time) * cur_buzzer_seq.num_of_beeps;

      // Turn buzzer on/off or increment index based on timing
      if(buzzer_time_delta >= beep_on_time_min && buzzer_time_delta < beep_on_time_max){
        digitalWrite(BUZZER_PIN, HIGH);
      } else if (buzzer_time_delta >= beep_off_time_min && buzzer_time_delta < beep_off_time_max){
        digitalWrite(BUZZER_PIN, LOW);
      } else if (buzzer_time_delta >= beep_off_time_max){
        buzzer_seq_index += 1;
      }

      if(buzzer_time_delta > beep_seq_time_max){
        buzzer_seq_on = false;
        buzzer_seq_index = 1;
      }
    }
  } else if(enabled){ // play single beep
    cur_time = millis();

    if((cur_time - buzzer_on_time) > current_buzzer_duration){
      digitalWrite(BUZZER_PIN, LOW);
      buzzer_on = false;
    }

  }
}
