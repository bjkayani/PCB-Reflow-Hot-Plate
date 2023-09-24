/**
  * PCB Reflow Hot Plate
  * Author: Badar Jahangir Kayani
  * Email: badarjahangir@gmail.com
  * Date: 4/10/2023
  * Platform: ESP32-C3-MINI-1-N4 base custom PCB
  * 
  * TODO:
  *   - Add WiFi support with configuration options
  *   - Add BLE support with configuration options
  *   - Add status icons for WiFi and BLE
  *   - Add option to print variables over serial
  *   - Settings menu: ( WiFi, BLE, USB Serial)
  *   - Add multiple temp sensor support
  *   - Add usb plug in/mode detection
  *   - Add timed pop-up alert support
  *   - Add scroll bar for all menus
  *   - Add option to change profile name
  */


#include "MAX6675.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "HWCDC.h"
#include <Preferences.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// ---------- IO Pin Definitions ----------

#define UP_BTN_PIN  0
#define OK_BTN_PIN  1
#define DN_BTN_PIN  4
#define BUZZER_PIN  20
#define SSR_PIN     10
#define SDA_PIN     2 // OLED Display
#define SCL_PIN     3 // OLED Display
#define DATA_PIN    7 // MAX6675
#define CLOCK_PIN   6 // MAX6675
#define SELECT_PIN  5 // MAX6675
#define RGB_LED_PIN 8

// ---------- Macros ----------

#define FW_VERSION            1.1
#define HW_VERSION            1.2
// Oled
#define OLED_SCREEN_ADDRESS   0x3C // I2C address
#define OLED_PIXEL_X          128
#define OLED_PIXEL_Y          64
// Button debounce
#define BUTTON_DEBOUNCE_TIME  20 // in mS
#define BUTTON_PRESS_TIME     100
// Temperature
#define TEMP_FILTER_WINDOW    10 // temp moving average window size
#define TEMP_UPDATE_INTERVAL  500 // temperature read interval in mS
#define MIN_TEMP              30
#define MAX_TEMP              260
#define SAFE_TEMP             70
// Setting change steps
#define SET_TEMP_STEP         5
#define PID_PARAMETER_STEP    0.1
#define REFLOW_TIME_STEP      10
#define REFLOW_TEMP_STEP      10
// SSR Relay PWM
#define PWM_FREQ              1 // in Hertz
#define PWM_PERIOD            (1000*(1/PWM_FREQ)) // Period in ms
#define PWM_MAX               100 // value for 100% PWM duty cycle
// Reflow profile
#define NUM_PROFILE_MAX       5 // stored profile capacity
#define PROFILE_MAX_POINTS    9  // time:Temperature point in each profile
// PID
#define PID_KD_PREGAIN        10.0
#define INTEG_ACTIVE_THRESH   80  // integral calc threshold to prevent windup
#define PID_FILTER_SIZE       10 // pid moving average window size
#define PID_STEP_INTERVAL     250 // PID update interval in mS
#define DERIV_CALC_INTERVAL   1000 // derivative calc interval for smoother response
// Buzzer
#define CLICK_BEEP            50   
#define SCROLL_BEEP           20
#define SCROLL_LIMIT_BEEP     100
#define START_UP_BEEP         200 
// Misc
#define MAX_REFLOW_TIME_S     600
#define NUM_SETTING_MAX       20
#define START_UP_SPLASH_TIME  2000
#define MAX_DISPLAY_LENGTH    20 
#define LED_BRIGHTNESS        150
// Error checking
#define BUFFER_TIME               30 // buffer time in seconds
#define BUFFER_SIZE               ((BUFFER_TIME * 1000) / ERROR_CHECK_INTERVAL)
#define ERROR_CHECK_INTERVAL      1000  // error checking interval in mS
#define HEAT_UP_MIN_TEMP_RISE     10  // minimal expected temp rise in BUFFER_TIME
#define HEAT_UP_CHECK_TEMP_LIMIT  200
#define HIGHTEMP_ERROR_LIMIT      280

// Switch between standard serial and usb serial
#define USB_DEBUG
#ifdef UART_DEBUG
  #define debugprint(x) Serial.print(x);
  #define debugprintln(x) Serial.println(x);
#endif
#ifdef USB_DEBUG
  #define debugprint(x) if(USBSerial.availableForWrite()){USBSerial.print(x);}
  #define debugprintln(x) if(USBSerial.availableForWrite()){USBSerial.println(x);}
#endif
#ifdef NO_DEBUG 
  #define debugprint(x)
  #define debugprintln(x)
#endif

// ---------- Objects ----------

MAX6675 thermoCouple;
Adafruit_SSD1306 display(OLED_PIXEL_X, OLED_PIXEL_Y, &Wire, -1);
Preferences preferences;
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;
Adafruit_NeoPixel pixels(1, RGB_LED_PIN, NEO_GRB + NEO_KHZ800);

// ---------- Enums and Structs ----------

enum main_select_t{
  MAIN_REFLOW = 0,
  MAIN_HEAT,
  MAIN_PARAM,
  MAIN_SETTINGS,
  MAIN_ABOUT,

  MAIN_NUM_ITEMS
};

enum parameter_select_t{
  PARAMETER_PID = 0,
  PARAMETER_REFLOW,

  PARAMETER_NUM_ITEMS
};

enum pid_select_t{
  PID_KP = 0,
  PID_KI,
  PID_KD,
  PID_KP_R,
  PID_KI_R,
  PID_KD_R,

  PID_NUM_ITEMS
};

enum reflow_select_t{
  REFLOW_TIME = 0,
  REFLOW_TEMP,

  REFLOW_NUM_ITEMS
};

enum single_button_state_t {
  BUTTON_RELEASED = 0,
  BUTTON_PRESSED = 1,
  BUTTON_NO_ACTION
};

enum buttons_state_t {
  BUTTONS_NO_PRESS,
  BUTTONS_UP_PRESS,
  BUTTONS_OK_PRESS,
  BUTTONS_DN_PRESS,
  BUTTONS_UPDN_PRESS
};

enum settings_state_t {
  SETTINGS_BUZZER = 0,
  SETTINGS_RESET,
  SETTINGS_TIMEOUT,
  
  SETTINGS_NUM_ITEMS  
};

enum op_mode_t {
  REFLOW = 0,
  HEAT
};

enum color_t {
    RED,
    GREEN,
    BLUE
};

/**
 * Struct to save reflow profiles
 * Point of change in the profile will be saved
 * Each point is saved as a time/temp pair
 * Time steps should be chronological until max time
 * Time steps subsequent from max time can be zero
 */
struct reflow_profile_t{
  int time_steps[PROFILE_MAX_POINTS];
  int temp_steps[PROFILE_MAX_POINTS];
  char display_name[MAX_DISPLAY_LENGTH];
};

struct pid_item_t{
  char display_name[MAX_DISPLAY_LENGTH];
  float val;
};

// Struct to store information for settings menu
struct settings_item_t{
  char display_name[MAX_DISPLAY_LENGTH]; // display name for the setting
  char display_options[NUM_SETTING_MAX][MAX_DISPLAY_LENGTH];  // display strings for each option
  int values[NUM_SETTING_MAX]; // integer value associated with each option
  int num_options;  // total number of options
  int set_value_index; // the currently selected option index
};

// Struct to store a buzzer tone sequence
struct buzzer_sequence_t{
  int buzzer_on_time; // time in ms for buzzer to be on
  int buzzer_off_time;  // time in ms for buzzer to be off
  int num_of_beeps; // number of times to play the on-off sequence
  bool enable_bypass; // play regradless of buzzer being disabled
};

// ---------- Constants ----------

const reflow_profile_t leaded_profile = { {0, 100, 200, 260, 280, 500, 0, 0, 0},
                                          {50, 150, 150, 220, 220, 50, 0, 0, 0},             
                                          "Leaded" };

// Reflow graph constants
const int graph_temp_min = 40;
const int graph_temp_max = 240;
const int graph_width = 100;
const int graph_height = 40;
const int graph_x_origin = 14;
const int graph_y_origin = 0;
const int left_tick_perc_array[4] = {20, 40, 60, 80};

// ---------- Variables ----------

// Buttons
volatile single_button_state_t up_button_state = BUTTON_NO_ACTION;
volatile single_button_state_t ok_button_state = BUTTON_NO_ACTION;
volatile single_button_state_t dn_button_state = BUTTON_NO_ACTION;
volatile unsigned long up_state_change_time = 0;
volatile unsigned long ok_state_change_time = 0;
volatile unsigned long dn_state_change_time = 0;
// System
float cur_temp = 30;
int heater_pwm = 0;
// Reflow
bool reflow_active = false;
int selected_profile = 0; 
bool reflow_done = false;
bool reflow_paused = false;
int reflow_duration_s = 0;
// Heat
bool heat_active = false;
float heat_set_temp = 0.0;
// Graph
int graph_time_min = 0;
int graph_time_max = 400;
// Buzzer
unsigned long buzzer_on_time = 0;
int current_buzzer_duration = 0;
bool buzzer_on = false;
buzzer_sequence_t cur_buzzer_seq;
bool buzzer_seq_on = false;
unsigned long buzzer_seq_on_time = 0;
int buzzer_seq_index = 1;
// Errors
bool high_temp_error = false;
bool heat_up_error = false;
// Circular buffer
float temp_buffer[BUFFER_SIZE];
int pwm_buffer[BUFFER_SIZE];
int buffer_head = 0;
int buffer_tail = 0;
int buffer_count = 0;

char print_buffer[20];

String mac_address; // unique hardware address

reflow_profile_t reflow_profile_array[NUM_PROFILE_MAX];    

// Should match main_select_t
char main_items [MAIN_NUM_ITEMS] [MAX_DISPLAY_LENGTH] = {
  "Reflow",
  "Heat",
  "Parameters",
  "Settings",
  "About"
};

// Should match parameter_select_t
char parameter_items[PARAMETER_NUM_ITEMS][MAX_DISPLAY_LENGTH] = {
  "PID",
  "Profile"
};

// Should match pid_select_t
pid_item_t pid_items[PID_NUM_ITEMS] = {
  {"Kp", 8.0},
  {"Ki", 0.3},
  {"Kd", 4.5},
  {"Kp_r", 8.0},
  {"Ki_r", 0.6},
  {"Kd_r", 3.5}
};

settings_item_t setting_items[SETTINGS_NUM_ITEMS] = {
  {"Buzzer",  {"On", "Off"},  {1, 0}, 2, 0},
  {"Reset",   {"", "Sure? ^"}, {0, 1}, 2, 0},
  {"Timeout", {"10m", "9m", "8m", "7m", "6m", "5m"}, {600, 540, 480, 420, 360, 300}, 6, 0}
};

int heat_timeout = setting_items[SETTINGS_TIMEOUT].values[setting_items[SETTINGS_TIMEOUT].set_value_index];

const buzzer_sequence_t buzzer_timeout_seq = {200, 200, 2, true};
const buzzer_sequence_t buzzer_reflow_complete_seq = {200, 200, 2, true};
const buzzer_sequence_t buzzer_high_temp_error_seq = {500, 500, 50, true};
const buzzer_sequence_t buzzer_heat_up_error_seq = {200, 200, 10, true};

// ---------- Control Functions ----------

/**
 * @brief Main menu control function
 */
void mainMenu(){

  static int select_index = 0;

  while(1){
    buttons_state_t cur_button = getButtonsState();

    /** 
    * Main menu button control
    * 
    * UP/DN   scroll the menu
    * OK      select the highlighted option
    */  
    switch(cur_button){
      case BUTTONS_UP_PRESS:
        if(--select_index < 0){
          select_index = 0;
          buzzerBeep(SCROLL_LIMIT_BEEP);
        } else {
          buzzerBeep(SCROLL_BEEP);
        }
        break;
      case BUTTONS_DN_PRESS:
        if(++select_index >= MAIN_NUM_ITEMS){
          select_index = MAIN_NUM_ITEMS - 1;
          buzzerBeep(SCROLL_LIMIT_BEEP);
        } else {
          buzzerBeep(SCROLL_BEEP);
        }
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeepBlocking(CLICK_BEEP);
        switch(select_index){
          case MAIN_REFLOW:
            reflow();
            break;
          case MAIN_HEAT:
            heat();
            break;
          case MAIN_PARAM:
            parameterMenu();
            break;
          case MAIN_SETTINGS:
            settings();
            break;
          case MAIN_ABOUT:
            about();
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    showMainMenu(select_index);
    buzzerLoop();
    ledSetColor(BLUE, LED_BRIGHTNESS);
  }

}

/**
 * @brief Reflow control function
 * This mode runs predefined reflow profiles
 */
void reflow(){

  static unsigned int reflow_time_ms = 0;
  static unsigned int reflow_time_s = 0;
  static unsigned int reflow_pre_pause_time_ms = 0;
  static unsigned long reflow_on_time = 0;
  static unsigned int reflow_progress = 0; // percentage completed
  static float reflow_set_temp = 0.0;

  while(1){
    cur_temp = readTempSensor();
    buttons_state_t cur_button = getButtonsState();
    reflow_duration_s = getReflowMaxTime();

    // Enter if reflow is actively running
    if(reflow_active && !reflow_paused && !high_temp_error){
      reflow_time_ms = (millis() - reflow_on_time) + reflow_pre_pause_time_ms; // get time elapsed 
      reflow_time_s = reflow_time_ms / 1000;
      reflow_progress = (reflow_time_s * 100) / reflow_duration_s;
      // Get the target temp from profile, run the pid step and set heater PWM
      reflow_set_temp = getReflowTemp(reflow_profile_array[selected_profile], reflow_time_s);
      heater_pwm = pidStep(reflow_set_temp, REFLOW);
    }

    // Enter if reflow is completed
    if(reflow_active && reflow_progress >= 100){
      reflow_done = true;  
      reflow_active = false; 
      heater_pwm = 0;
      buzzerSeq(buzzer_reflow_complete_seq);
      debugprintln("reflow completed"); 
    }

    // Handle high temperature error
    if(high_temp_error && reflow_active){
      reflow_active = false;
      heater_pwm = 0;
      buzzerSeq(buzzer_high_temp_error_seq);  
    }

    // Handle heater error
    if(heat_up_error && reflow_active){
      reflow_active = false;
      heater_pwm = 0;
      buzzerSeq(buzzer_heat_up_error_seq); 
    }

    /** 
    * Reflow button control
    * 
    * UP/DN   change reflow profile selected
    * OK      start/pause/resume reflow
    * UP+DN   stop reflow and exit
    */        
    switch(cur_button){
      case BUTTONS_UP_PRESS:
        if(!reflow_active && !reflow_done && !heat_up_error && !high_temp_error){
          if(--selected_profile < 0){
            selected_profile = 0;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
          updateMiscParams();
        }
        break;
      case BUTTONS_DN_PRESS:
        if(!reflow_active && !reflow_done && !heat_up_error && !high_temp_error){
          if(++selected_profile >= NUM_PROFILE_MAX){
            selected_profile = NUM_PROFILE_MAX - 1;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          } 
          updateMiscParams();
        }
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeep(CLICK_BEEP);
        if(reflow_active && !reflow_paused) { // pause reflow if its running
          reflow_paused = true;
          reflow_pre_pause_time_ms = reflow_time_ms;
          heater_pwm = 0;
          debugprintln("reflow paused");
        } else if (reflow_active && reflow_paused){ // resume reflow if paused
          reflow_paused = false; 
          reflow_on_time = millis();
        } else if (!reflow_done && !heat_up_error && !high_temp_error) {  // start reflow
          reflow_active = true;
          reflow_on_time = millis();
          reflow_time_s = 0;
          reflow_done = false;
          reflow_pre_pause_time_ms = 0;
          reflow_progress = 0;
          debugprintln("reflow started");
        }
        break;
      default:
        break;
    }

    // Reset reflow tracking variables and exit from reflow mode
    if(cur_button == BUTTONS_UPDN_PRESS){
      reflow_active = false;
      reflow_paused = false;
      reflow_done = false;
      high_temp_error = false;
      heat_up_error = false;
      reflow_progress = 0;
      heater_pwm = 0;
      heaterPwmLoop();  // called to set PWM before exiting
      debugprintln("reflow stopped");
      break;
    } 

    showReflow(reflow_set_temp, reflow_time_s, reflow_progress);
    buzzerLoop();
    errorCheckLoop(reflow_set_temp, reflow_active);
    heaterPwmLoop();
    ledHeatDisplay(cur_temp, LED_BRIGHTNESS);
  }
}

/**
 * @brief Heat mode control function
 * This mode heats the plate to a static temperature
 */
void heat(){

  static unsigned int heat_time_ms = 0;
  static unsigned int heat_time_s = 0;
  static unsigned long heat_on_time = 0;

  while(1){
    cur_temp = readTempSensor();
    buttons_state_t cur_button = getButtonsState();

    // Enter if heating has been turned on by the user
    if(heat_active){
      heat_time_ms = millis() - heat_on_time;
      heat_time_s = heat_time_ms / 1000;
      // Run the PID step and set heater PWM
      heater_pwm = pidStep(heat_set_temp, HEAT);
    }

    // Heating timed out
    if(heat_time_s > heat_timeout && heat_active){
      heat_active = false;
      heater_pwm = 0;
      buzzerSeq(buzzer_timeout_seq);
    }

    // Handle high temperature error
    if(high_temp_error && heat_active){
      heat_active = false;
      heater_pwm = 0;
      buzzerSeq(buzzer_high_temp_error_seq);  
    }

    // Handle heater error
    if(heat_up_error && heat_active){
      heat_active = false;
      heater_pwm = 0;
      buzzerSeq(buzzer_heat_up_error_seq); 
    }

    /** 
    * Heat button control
    * 
    * UP/DN   change heat set temperature
    * OK      start/stop heating
    * UP+DN   stop heating and exit
    */  
    switch(cur_button){
      case BUTTONS_UP_PRESS:
        heat_set_temp += SET_TEMP_STEP;
        if(heat_set_temp > MAX_TEMP){
          heat_set_temp = MAX_TEMP;
          buzzerBeep(SCROLL_LIMIT_BEEP);
        } else {
          buzzerBeep(SCROLL_BEEP);
        }
        updateMiscParams();
        break;
      case BUTTONS_DN_PRESS:
        heat_set_temp -= SET_TEMP_STEP;
        if(heat_set_temp < MIN_TEMP){
          heat_set_temp = MIN_TEMP;
          buzzerBeep(SCROLL_LIMIT_BEEP);
        } else {
          buzzerBeep(SCROLL_BEEP);
        }
        updateMiscParams();
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeep(CLICK_BEEP);
        if(heat_active) {
          heat_active = false;
          heater_pwm = 0;
          debugprintln("heating stopped");
        } else if (!heat_up_error && !high_temp_error) {
          heat_active = true;
          heat_on_time = millis();
          debugprintln("heating started");
        }
        break;
      default:
        break;
    }

    if(cur_button == BUTTONS_UPDN_PRESS){
      heat_active = false;
      high_temp_error = false;
      heat_up_error = false;
      heater_pwm = 0;
      heaterPwmLoop();  // called to set PWM before exiting
      debugprintln("heating stopped");
      break;
    } 

    showHeat(heat_time_s);
    buzzerLoop();
    errorCheckLoop(heat_set_temp, heat_active);
    heaterPwmLoop();
    ledHeatDisplay(cur_temp, LED_BRIGHTNESS);
  }
}

/**
 * @brief Parameter menu control function
 */
void parameterMenu(){
  static int select_index = 0;

  while(1){
    buttons_state_t cur_button = getButtonsState();

    /** 
    * Parameter menu button control
    * 
    * UP/DN   scroll the menu
    * OK      select the highlighted option
    * UP+DN   exit
    */  
    switch(cur_button){
      case BUTTONS_UP_PRESS:  
        if(--select_index < 0){
          select_index = 0;
          buzzerBeep(SCROLL_LIMIT_BEEP);
        } else {
          buzzerBeep(SCROLL_BEEP);
        }
        break;
      case BUTTONS_DN_PRESS:
        if(++select_index >= PARAMETER_NUM_ITEMS){
          select_index = PARAMETER_NUM_ITEMS - 1;
          buzzerBeep(SCROLL_LIMIT_BEEP);
        } else {
          buzzerBeep(SCROLL_BEEP);
        }
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeepBlocking(CLICK_BEEP);
        switch(select_index){
          case PARAMETER_PID:
            pidParameter();
            break;
          case PARAMETER_REFLOW:
            profileParameter();
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    if(cur_button == BUTTONS_UPDN_PRESS){
      break;
    } 

    showParameterMenu(select_index);
    buzzerLoop();
  }

}

/**
 * @brief PID parameter menu control function
 * Menu to change PID parameters
 */
void pidParameter(){

  static int select_index = 0;
  static bool selected = false;
  
  while(1){
    buttons_state_t cur_button = getButtonsState();

    /** 
    * PID parameter menu button control
    * 
    * UP/DN   scroll through paramters, increase/decrease selected parameter
    * OK      select the highlighted option
    * UP+DN   exit
    */  
    switch(cur_button){
      case BUTTONS_UP_PRESS:
        if (selected){  // change parameter value if selected
          pid_items[select_index].val += PID_PARAMETER_STEP;
          updatePidParams(select_index);
        } else { // move to prev option otherwise
          if(--select_index < 0){
            select_index = 0;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
        }
        break;
      case BUTTONS_DN_PRESS:
        if (selected){  // change parameter value if selected
          pid_items[select_index].val -= PID_PARAMETER_STEP;
          updatePidParams(select_index);
        } else {  // move to next option otherwise
          if(++select_index >= PID_NUM_ITEMS){
            select_index = PID_NUM_ITEMS - 1;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
        }
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeep(CLICK_BEEP);
        selected = !selected;
        break;
      default:
        break;
    }

    if(cur_button == BUTTONS_UPDN_PRESS){
      selected = false;
      break;
    } 

    showPidParameters(select_index, selected);
    buzzerLoop();
  }
}

/**
 * @brief Profile parameter menu control function
 * Menu to change reflow profile time/temp points
 * 
 * TODO: Add profile validation function
 */
void profileParameter(){

  static int select_index = 0;
  static bool selected = false;
  static int sub_select = REFLOW_TIME;

  while(1){
    buttons_state_t cur_button = getButtonsState();

    /** 
    * Reflow profile parameter menu button control
    * 
    * UP/DN   scroll through parameters, increase/decrease selected parameter
    * OK      select the highlighted option
    * UP+DN   exit
    * 
    * Extra logic to deal with the pinned reflow profile name and time/temp selection
    */ 
    switch(cur_button){
      case BUTTONS_UP_PRESS:
        if (selected){
          if(select_index == 0){  // change profile selected if on profile name
            if(--selected_profile < 0){
              selected_profile = 0;
            }
            updateMiscParams();
          } else {  // change value of selected parameter
            if(sub_select == REFLOW_TIME){ // sub selection: unselected -> time -> temp
              reflow_profile_array[selected_profile].time_steps[select_index-1] += REFLOW_TIME_STEP;
            } else {
              reflow_profile_array[selected_profile].temp_steps[select_index-1] += REFLOW_TEMP_STEP;
            }
          }
          updateReflowProfile(selected_profile);
        } else {
          if(--select_index < 0){
            select_index = 0;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
        }
        break;
        break;
      case BUTTONS_DN_PRESS:
        if (selected){
          if(select_index == 0){
            if(++selected_profile >= NUM_PROFILE_MAX){
              selected_profile = NUM_PROFILE_MAX -1;
            }
            updateMiscParams();
          } else {
            if(sub_select == REFLOW_TIME){
              reflow_profile_array[selected_profile].time_steps[select_index-1] -= REFLOW_TIME_STEP;
            } else {
              reflow_profile_array[selected_profile].temp_steps[select_index-1] -= REFLOW_TEMP_STEP;
            }
          }
        } else {
          if(++select_index >= (PROFILE_MAX_POINTS + 1)  && !selected){
            select_index = PROFILE_MAX_POINTS;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
        }
        updateReflowProfile(selected_profile);
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeep(CLICK_BEEP);
        if(selected){
          if(sub_select == REFLOW_TIME && select_index != 0){
            sub_select = REFLOW_TEMP;
          } else {
            selected = false;
          }
        } else {
          selected = true;
          sub_select = REFLOW_TIME;
        }
        break;
      default:
        break;
    }

    if(cur_button == BUTTONS_UPDN_PRESS){
      selected = false;
      break;
    } 

    showProfileParameters(select_index, selected, sub_select);
    buzzerLoop();
  }

}

/**
 * @brief Settings menu control function
 * Menu to change system settings
 * Data stored in setting_item array defines parameters for each setting
 */
void settings(){

  static int select_index = 0;
  static bool selected = false;

  while(1){
    buttons_state_t cur_button = getButtonsState();

    /** 
    * Settings menu button control
    * 
    * UP/DN   scroll through settings, scroll through options for each setting
    * OK      select the highlighted option
    * UP+DN   exit
    * 
    * Extra logic to deal with the system reset option
    */ 
    switch(cur_button){
      case BUTTONS_UP_PRESS:
        if (selected){
          // Reset system if clicked UP when reset is primed
          if(select_index == SETTINGS_RESET){ 
            if(setting_items[SETTINGS_RESET].set_value_index == 1){
              reset();
            }
          }
          // Otherwise change setting value as usual
          if(--setting_items[select_index].set_value_index < 0){
            setting_items[select_index].set_value_index = 0;
          }
          updateSettings(); // save the new settings
        } else { // scroll through menu if not selected
          if(--select_index < 0){
            select_index = 0;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
        }
        break;
      case BUTTONS_DN_PRESS:
        if(selected){
          // Increment the set_value_index of the currently selected setting_item
          // Limit its value base on num_options
          if(++setting_items[select_index].set_value_index >= setting_items[select_index].num_options){
            setting_items[select_index].set_value_index = setting_items[select_index].num_options - 1;
          }
          updateSettings();
        } else {
          if(++select_index >= SETTINGS_NUM_ITEMS){
            select_index = SETTINGS_NUM_ITEMS - 1;
            buzzerBeep(SCROLL_LIMIT_BEEP);
          } else {
            buzzerBeep(SCROLL_BEEP);
          }
        }
        break;
      case BUTTONS_OK_PRESS:
        buzzerBeep(CLICK_BEEP);
        selected = !selected;
        // Prime reset if selected
        if(select_index == SETTINGS_RESET){
          if(++setting_items[SETTINGS_RESET].set_value_index >= setting_items[SETTINGS_RESET].num_options){
            setting_items[SETTINGS_RESET].set_value_index = 0;
          }
        }
        break;
      default:
        break;
    }

    // Unprime reset and exit
    if(cur_button == BUTTONS_UPDN_PRESS){
      setting_items[SETTINGS_RESET].set_value_index = 0;
      selected = false;
      break;
    } 

    showSettings(select_index, selected);
    buzzerLoop();
  }


}

/**
 * @brief About screen control function
 * TODO: Add more information and scrolling ability
 */
void about(){

  while(1){

    buttons_state_t cur_button = getButtonsState();

    if(cur_button == BUTTONS_UPDN_PRESS){
      break;
    } 

    showAbout();
  }
}


/**
 * @brief Reset function
 * Factory reset parameters and restart unit
 */
void reset(){
  preferences.clear();
  ESP.restart();
}

/**
 * @brief Get the MAC Address
 * 
 */
void getMacAddress(){
  mac_address = WiFi.macAddress();
  mac_address.replace(":", "");
}

void setup() {
  #ifdef UART_DEBUG
    Serial.begin(115200);
  #endif
  #ifdef USB_DEBUG
    USBSerial.begin(115200);
  #endif

  debugprintln("---------- PCB Reflow Hot Plate ----------");
  debugprint("Initializing...");

  // OLED display init
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    Serial.println("OLED Display Init Failed");
  }
  u8g2_for_adafruit_gfx.begin(display);  
  display.clearDisplay();

  // Thermocouple init
  thermoCouple.begin(CLOCK_PIN, SELECT_PIN, DATA_PIN);
  thermoCouple.setSPIspeed(4000000);

  // RGB LED init
  pixels.begin();

  // I/O init
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(OK_BTN_PIN, INPUT);
  pinMode(DN_BTN_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SSR_PIN, OUTPUT);

  // Button ISR init
  attachInterrupt(UP_BTN_PIN, up_change_isr, RISING);
  attachInterrupt(OK_BTN_PIN, ok_change_isr, RISING);
  attachInterrupt(DN_BTN_PIN, dn_change_isr, RISING);

  // Saved parameter init
  preferences.begin("params", false);
  initMiscParams();
  initPidParams();
  initReflowProfiles();
  initSettings();

  getMacAddress();

  debugprintln(" Initialization completed.");

  buzzerBeepBlocking(START_UP_BEEP);
  showSplashScreen(START_UP_SPLASH_TIME);
  
  mainMenu(); // entry point

}

void loop() { // never goes here
}
