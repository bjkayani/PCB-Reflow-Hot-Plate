/**
 * PCB Reflow Hot Plate
 * buttons.ino 
 * Contains function to read the buttons
 */

/**
 * @brief Get button state
 * Needs to be called in a fast loop to catch key presses
 * Buttons trigger ISR's that set button press time
 * This function uses the timing to determine button presses
 * @return buttons_state_t state of the buttons
 * 
 * TODO: Add more button combinations
 * TODO: Add activate on release
 * TODO: Add long press detection
 */
buttons_state_t getButtonsState(){
  single_button_state_t button_dn;
  single_button_state_t button_ok;
  single_button_state_t button_up;
  unsigned long button_dn_time;
  unsigned long button_ok_time;
  unsigned long button_up_time;

  // Get button states set by interrupts 
  noInterrupts();
  button_up = up_button_state;
  button_ok = ok_button_state;
  button_dn = dn_button_state;
  button_up_time = up_state_change_time;
  button_ok_time = ok_state_change_time;
  button_dn_time = dn_state_change_time;
  interrupts();

  unsigned long cur_time = millis();
  buttons_state_t state = BUTTONS_NO_PRESS;

  // Check for debounce and clear any erroneous button states
  if(button_up == BUTTON_PRESSED && 
     cur_time - up_state_change_time > BUTTON_DEBOUNCE_TIME){
      if(digitalRead(UP_BTN_PIN) != BUTTON_PRESSED){
        noInterrupts();
        up_button_state = BUTTON_NO_ACTION;
        button_up = BUTTON_NO_ACTION;
        interrupts();
      }   
  }

  if(button_ok == BUTTON_PRESSED && 
     cur_time - ok_state_change_time > BUTTON_DEBOUNCE_TIME){
      if(digitalRead(OK_BTN_PIN) != BUTTON_PRESSED){
        noInterrupts();
        ok_button_state = BUTTON_NO_ACTION;
        button_ok = BUTTON_NO_ACTION;
        interrupts();
      }   
  }

  if(button_dn == BUTTON_PRESSED && 
     cur_time - dn_state_change_time > BUTTON_DEBOUNCE_TIME){
      if(digitalRead(DN_BTN_PIN) != BUTTON_PRESSED){
        noInterrupts();
        dn_button_state = BUTTON_NO_ACTION;
        button_dn = BUTTON_NO_ACTION;
        interrupts();
      }   
  }

  // Check for button press and clear state for next button event
  if (button_dn == BUTTON_PRESSED && button_up == BUTTON_PRESSED) { // UP and DN button pressed together
      long up_dn_time_delta = button_dn_time - button_up_time; // verify this
      if(abs(up_dn_time_delta) < BUTTON_PRESS_TIME){  // if the two are pressed somewhat together
        if (cur_time - button_dn_time > BUTTON_PRESS_TIME &&
            cur_time - button_up_time > BUTTON_PRESS_TIME) {
          state = BUTTONS_UPDN_PRESS;
          noInterrupts();
          dn_button_state = BUTTON_NO_ACTION;
          up_button_state = BUTTON_NO_ACTION;
          interrupts();
        }
      }
  } else if (button_up == BUTTON_PRESSED &&
             cur_time - button_up_time > BUTTON_PRESS_TIME) { // UP button pressed
    state = BUTTONS_UP_PRESS;
    noInterrupts();
    up_button_state = BUTTON_NO_ACTION;
    interrupts();
  } else if (button_ok == BUTTON_PRESSED &&
             cur_time - button_ok_time > BUTTON_PRESS_TIME) { // DN button pressed
    state = BUTTONS_OK_PRESS;
    noInterrupts();
    ok_button_state = BUTTON_NO_ACTION;
    interrupts();
  } else if (button_dn == BUTTON_PRESSED &&
             cur_time - button_dn_time > BUTTON_PRESS_TIME) { // OK button pressed
    state = BUTTONS_DN_PRESS;
    noInterrupts();
    dn_button_state = BUTTON_NO_ACTION;
    interrupts();
  }

  return state;

}

// ISR to capture button press
// State will be reset by getButtonState()

/**
 * @brief Up button change ISR
 */
void up_change_isr() {
  up_button_state = BUTTON_PRESSED;
  up_state_change_time = millis();
}

/**
 * @brief Ok button change ISR
 */
void ok_change_isr() {
  ok_button_state = BUTTON_PRESSED;
  ok_state_change_time = millis();
}

/**
 * @brief Down button change ISR
 */
void dn_change_isr() {
  dn_button_state = BUTTON_PRESSED;
  dn_state_change_time = millis();
}