/**
 * PCB Reflow Hot Plate
 * display.ino 
 * Contains all OLED display functions
 * Font details can be found at: https://github.com/olikraus/u8g2/wiki/fntlistallplain
 */

/**
 * @brief Main menu display function
 * @param index array index of selected item from main menu list
 * TODO: Combine simple menu style screens
 */
void showMainMenu(int index){
  static int select_index = 0;
  static int display_rows[3] = {0, 1, 2};

  // Update items to display based on user interaction
  if (index == display_rows[0]){ // Selection moved to top row
    select_index = 0;
  } else if(index == display_rows[1]){ // Selection moved to middle row
    select_index = 1;
  } else if (index == display_rows[2]){ // Selection moved to bottom row
    select_index = 2;
  } else if (index > display_rows[2] && index < MAIN_NUM_ITEMS){ // Move select options one row down if not at end
    display_rows[0] += 1;
    display_rows[1] += 1;
    display_rows[2] += 1;
  } else if (index < display_rows[0] && index > -1){ // Move select options one row up if not at top
    display_rows[0] -= 1;
    display_rows[1] -= 1;
    display_rows[2] -= 1;
  }

  display.clearDisplay();
  u8g2_for_adafruit_gfx.setForegroundColor(WHITE); 
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox3hb_tf);

  // Display items and draw rectangle around selected item
  for(int i = 0; i < 3; i++){
    if(i == select_index){
      display.drawRoundRect(0, (i * 21), 128, 22, 5, SSD1306_WHITE);
    }
    u8g2_for_adafruit_gfx.setCursor(5, 16 + (i * 21));
    u8g2_for_adafruit_gfx.print(main_items[display_rows[i]]);
  }

  display.display();

}

/**
 * @brief Parameter menu display function
 * @param index array index of selected item from parameter menu list
 */
void showParameterMenu(int index){
  static int select_index = 0;
  static int display_rows[3] = {0, 1, 2};

  if (index == display_rows[0]){ // Selection moved to top row
    select_index = 0;
  } else if(index == display_rows[1]){ // Selection moved to middle row
    select_index = 1;
  } else if (index == display_rows[2]){ // Selection moved to bottom row
    select_index = 2;
  } else if (index > display_rows[2] && index < PARAMETER_NUM_ITEMS){ // Move select options one row down if not at end
    display_rows[0] += 1;
    display_rows[1] += 1;
    display_rows[2] += 1;
  } else if (index < display_rows[0] && index > -1){ // Move select options one row up if not at top
    display_rows[0] -= 1;
    display_rows[1] -= 1;
    display_rows[2] -= 1;
  }

  display.clearDisplay();
  u8g2_for_adafruit_gfx.setForegroundColor(WHITE); 
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox3hb_tf);

  for(int i = 0; i < min(3, (int)PARAMETER_NUM_ITEMS); i++){
    if(i == select_index){
      display.drawRoundRect(0, (i * 21), 128, 22, 5, SSD1306_WHITE);
    }
    u8g2_for_adafruit_gfx.setCursor(5, 16 + (i * 21));
    u8g2_for_adafruit_gfx.print(parameter_items[display_rows[i]]);
  }

  display.display();

}

/**
 * @brief Reflow graph drawing function
 * Draws static graph for selected profile before reflow is started
 * Draws a dynamic graph using actual temperature during reflow process
 * @param time time in seconds to plot for dynamic plotting
 * @param temp current temperature for dynamic plotting
 * TODO: Abstract out the graph drawing as a function
 */
void showReflowGraph(int time, int temp){
  static int graph_array[graph_width][2]; // pixel location array
  static int last_graph_x = 0;
  static int last_time_s = 0;
  static int preview_temp = 0;
  static int preview_index = 0;
  static int graph_data_index = 0; // index for pixel tracking

  graph_time_max = getReflowMaxTime(); // get maximum time from reflow profile

  display.drawRect(graph_x_origin, graph_y_origin, graph_width, graph_height, SSD1306_WHITE);
  
  // Draw Y ticks and tick values
  u8g2_for_adafruit_gfx.setFont(u8g2_font_u8glib_4_tf);
  for(int i = 0; i < sizeof(left_tick_perc_array)/sizeof(int); i++){
    int perc = left_tick_perc_array[i];
    sprintf(print_buffer, "%d", ((perc * (graph_temp_max - graph_temp_min)) / 100) + graph_temp_min);
    rightText(print_buffer, 11, getYTickPixel(perc) + 2);
    display.drawFastHLine(graph_x_origin, getYTickPixel(perc), 3, SSD1306_WHITE);
  }

  // Draw static graph of selected profile prior to reflow
  if(!reflow_active && !reflow_done){
    u8g2_for_adafruit_gfx.setFontMode(0);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_4x6_tf);
    centeredText("Start Reflow [OK]", 64, (graph_height + graph_y_origin - 2));

    // Iterate over reflow profile and calculate equivalent graphing points
    for(int t = 0; t < graph_time_max; t++){
      preview_temp = getReflowTemp(reflow_profile_array[selected_profile], t); // get temperature at each time step
      preview_temp = constrain(preview_temp, graph_temp_min, graph_temp_max);

      // Map each time/temp point from profile to equivalent pixel location on the OLED display
      graph_array[graph_data_index][0] = map(t, graph_time_min, graph_time_max, 0, graph_width);
      graph_array[graph_data_index][0] += graph_x_origin;
      graph_array[graph_data_index][1] = map(preview_temp, graph_temp_min, graph_temp_max, graph_height, 0);
      graph_array[graph_data_index][1] += (graph_y_origin - 1);

      // Increment to next point on display if it maps to a new pixel location
      if(graph_array[graph_data_index][0] != last_graph_x){ 
        last_graph_x = graph_array[graph_data_index][0];
        if(++graph_data_index >= graph_width){ // Safe increment index for pixel location array
          graph_data_index = graph_width - 1;
        }  
      }
    }

    // Display all pixels in the pixel location array
    for(int i = 0; i < graph_data_index; i++){
      display.writePixel(graph_array[i][0], graph_array[i][1], SSD1306_WHITE);
    }

    // Reset dynamic graphing parameters for next run
    last_graph_x = 0;    
    last_time_s = 0;
    graph_data_index = 0;
  } else { 
    // Draw dynamic graph as the reflow profile is executed

    // Constrain inputs to avoid drawing outside of graph area
    time = constrain(time, graph_time_min, graph_time_max); 
    temp = constrain(temp, graph_temp_min, graph_temp_max);

    // Calculate next pixel point after 1 second
    if(time > last_time_s){
      // Map the time and temp to the display pixel
      graph_array[graph_data_index][0] = map(time, graph_time_min, graph_time_max, 0, graph_width);
      graph_array[graph_data_index][0] += graph_x_origin;
      graph_array[graph_data_index][1] = map(temp, graph_temp_min, graph_temp_max, graph_height, 0);
      graph_array[graph_data_index][1] += (graph_y_origin - 1);

      // Increment to next point on display if it maps to a new pixel location
      if(last_graph_x != graph_array[graph_data_index][0]){
        last_graph_x = graph_array[graph_data_index][0];
        if(++graph_data_index >= graph_width){ // Safe increment
          graph_data_index = graph_width - 1;
        }  
        
      }
      last_time_s = time;
    }

    // Display graph pixels populated uptil now 
    for(int i = 0; i < graph_data_index; i++){
      display.writePixel(graph_array[i][0], graph_array[i][1], SSD1306_WHITE);
    }
  }

}

/**
 * @brief Reflow display function
 * @param set_temp current target temperature from the reflow profile
 * @param reflow_time time elapsed in the reflow profile
 * @param progress percentage of profile completed
 */
void showReflow(float set_temp, int reflow_time, int progress){

  static int display_temp = 0;

  display_temp = round(cur_temp); // round the current temperature to nearest int

  display.clearDisplay();
  showReflowGraph(reflow_time, display_temp); // display the graph

  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2hb_tf);
  sprintf(print_buffer, "%d°C", display_temp);
  rightText(print_buffer, 106, 60); // display current temperature

  // Display status icon and percentage if reflow is active
  if(reflow_active){
    display.fillRoundRect(0, 46, 20, 18, 5, SSD1306_WHITE);
    u8g2_for_adafruit_gfx.setForegroundColor(INVERSE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_open_iconic_play_2x_t);
    u8g2_for_adafruit_gfx.setFontMode(1); 

    // Display play or paused icons based on status
    if(reflow_paused){
      u8g2_for_adafruit_gfx.setCursor(2, 63);
      u8g2_for_adafruit_gfx.print("D");
    } else {
      u8g2_for_adafruit_gfx.setCursor(3, 63);
      u8g2_for_adafruit_gfx.print("E");
    }

    // Display percentage completed
    u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2hb_tf);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setCursor(25, 60);
    sprintf(print_buffer, "%d%%", progress);
    u8g2_for_adafruit_gfx.print(print_buffer);
    
    // Display sun icon if heater is on
    if(digitalRead(SSR_PIN)){
      u8g2_for_adafruit_gfx.setFont(u8g2_font_open_iconic_weather_2x_t);
      u8g2_for_adafruit_gfx.setCursor(111, 63);
      u8g2_for_adafruit_gfx.print("E");
    }
  } else { 
    // Enter here if reflow is inactive

    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2hb_tf);

    //Display done or selected profile name 
    if(reflow_done){
      u8g2_for_adafruit_gfx.setCursor(5, 60);
      sprintf(print_buffer, "Done");     
    } else {
      u8g2_for_adafruit_gfx.setCursor(5, 60);
      sprintf(print_buffer, "%s", reflow_profile_array[selected_profile].display_name);
    }
    u8g2_for_adafruit_gfx.print(print_buffer); 
  }

  display.display();
}


/**
 * @brief Heat mode display function
 * @param on_time time elapsed since heating was started 
 */
void showHeat(int on_time){
  static unsigned int on_duration_m = 0;
  static unsigned int on_display_s = 0;

  // Calculate mins and seconds
  on_duration_m = on_time / 60;
  on_display_s = on_time % 60;

  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox5h_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 17);
  sprintf(print_buffer, "%.1f°C", cur_temp);  // display current temperature
  u8g2_for_adafruit_gfx.print(print_buffer);

  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2h_tf);
  u8g2_for_adafruit_gfx.setCursor(0, 35);
  sprintf(print_buffer, "T-Set: %.0f°C", heat_set_temp);  // dispaly target temperature
  u8g2_for_adafruit_gfx.print(print_buffer);

  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox3hb_tf);
  if(heat_active){
    
    display.fillRoundRect(0, 42, 40, 21, 5, SSD1306_WHITE);
    u8g2_for_adafruit_gfx.setForegroundColor(INVERSE);
    u8g2_for_adafruit_gfx.setFontMode(1); 
    u8g2_for_adafruit_gfx.setCursor(7, 58);
    u8g2_for_adafruit_gfx.print("On");  // Display "On" if heating is active

    // Display time since heating was started
    sprintf(print_buffer, "%02d:%02d", on_duration_m, on_display_s);
    u8g2_for_adafruit_gfx.setCursor(45, 58);
    u8g2_for_adafruit_gfx.print(print_buffer);
    
    // Display sun icon if heater is currently on
    if(digitalRead(SSR_PIN)){
      u8g2_for_adafruit_gfx.setFont(u8g2_font_open_iconic_weather_2x_t);
      u8g2_for_adafruit_gfx.setCursor(111, 63);
      u8g2_for_adafruit_gfx.print("E");
    }
  } else {
    // Display "Off" if heating is inactive
    display.drawRoundRect(0, 42, 40, 21, 5, SSD1306_WHITE);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setCursor(6, 58);
    u8g2_for_adafruit_gfx.print("Off");
  }

  display.display();

}

/**
 * @brief PID parameter menu display function
 * Uses similar display logic as the main menu function
 * @param index currently highlighted option
 * @param selected if highlighted option is selected 
 */
void showPidParameters(int index, bool selected){
  static int select_index = 0;
  static int display_rows[4] = {0, 1, 2, 3};

  if (index == display_rows[0]){ // Selection moved to top row
    select_index = 0;
  } else if(index == display_rows[1]){ // Selection moved to middle row
    select_index = 1;
  } else if (index == display_rows[2]){ // Selection moved to bottom row
    select_index = 2;
  } else if (index == display_rows[3]){ // Selection moved to bottom row
    select_index = 3;
  } else if (index > display_rows[3] && index < PID_NUM_ITEMS){ // Move select options one row down if not at end
    display_rows[0] += 1;
    display_rows[1] += 1;
    display_rows[2] += 1;
    display_rows[3] += 1;
  } else if (index < display_rows[0] && index > -1){ // Move select options one row up if not at top
    display_rows[0] -= 1;
    display_rows[1] -= 1;
    display_rows[2] -= 1;
    display_rows[3] -= 1;
  }

  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2hb_tf);
  u8g2_for_adafruit_gfx.setFontMode(1); 
  for(int i = 0; i < 4; i++){
    // Invert colors if the highlighted row is selected
    if(i == select_index){
      if(selected){
        display.fillRoundRect(0, (i * 16), 128, 16, 5, SSD1306_WHITE);
        u8g2_for_adafruit_gfx.setForegroundColor(INVERSE); 
      } else {
        display.drawRoundRect(0, (i * 16), 128, 16, 5, SSD1306_WHITE);
        u8g2_for_adafruit_gfx.setForegroundColor(WHITE); 
      }
    }

    u8g2_for_adafruit_gfx.setCursor(5, 13 + (i * 16));
    sprintf(print_buffer, "%s: %1.1f", pid_items[display_rows[i]].display_name, 
                                      pid_items[display_rows[i]].val);
    u8g2_for_adafruit_gfx.print(print_buffer);
  }

  display.display();
}

/**
 * @brief Reflow profile menu display function
 * Displays profile name pinned to top
 * Time/Temp points are scrollable
 * Selection rotates between time and temp
 * @param index index of highlighted option
 * @param selected if highlighted option is selected
 * @param sub_select time or temp selection
 */
void showProfileParameters(int index, bool selected, int sub_select){

  static int select_index = 0;
  static int display_rows[3] = {1, 2, 3};

  display.clearDisplay();  
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2hb_tf);
  u8g2_for_adafruit_gfx.setFontMode(1); 

  // Apply item display logic if highlighted item isnt the profile name
  if(index > 0){
    if (index == display_rows[0]){ // Selection moved to top row
      select_index = 0;
    } else if(index == display_rows[1]){ // Selection moved to middle row
      select_index = 1;
    } else if (index == display_rows[2]){ // Selection moved to bottom row
      select_index = 2;
    } else if (index > display_rows[2] && index < PROFILE_MAX_POINTS + 1){ // Move select options one row down if not at end
      display_rows[0] += 1;
      display_rows[1] += 1;
      display_rows[2] += 1;
    } else if (index < display_rows[0] && index > -1){ // Move select options one row up if not at top
      display_rows[0] -= 1;
      display_rows[1] -= 1;
      display_rows[2] -= 1;
    }
  }

  for(int i = 0; i < 3; i++){

    // Profile name highlight and select display
    if(index == 0 && selected){ 
      display.fillRoundRect(0, 0, 128, 16, 5, SSD1306_WHITE);
      u8g2_for_adafruit_gfx.setForegroundColor(INVERSE); 
      u8g2_for_adafruit_gfx.setCursor(5, 13);
    } else if (index == 0) {
      display.drawRoundRect(0, 0, 128, 16, 5, SSD1306_WHITE);
      u8g2_for_adafruit_gfx.setForegroundColor(WHITE); 
    }  

    // Display name of current profile
    u8g2_for_adafruit_gfx.setCursor(5, 13);
    sprintf(print_buffer, "Profile: %s", reflow_profile_array[selected_profile].display_name);
    u8g2_for_adafruit_gfx.print(print_buffer);
    
    // Highlight and select display for parameters
    if(i == select_index && index > 0){
      if(selected){
        if(sub_select == REFLOW_TIME){ // Select the time
          display.fillRoundRect(20, ((i+1) * 16), 40, 16, 5, SSD1306_WHITE);
          u8g2_for_adafruit_gfx.setForegroundColor(INVERSE); 
        } else if (sub_select == REFLOW_TEMP){ // Select the temp
          display.fillRoundRect(60, ((i+1) * 16), 50, 16, 5, SSD1306_WHITE);
          u8g2_for_adafruit_gfx.setForegroundColor(INVERSE); 
        }
      } else {
        display.drawRoundRect(0, ((i+1) * 16), 128, 16, 5, SSD1306_WHITE);
        u8g2_for_adafruit_gfx.setForegroundColor(WHITE); 
      }
    }

    u8g2_for_adafruit_gfx.setCursor(5, 13 + ((i + 1) * 16));
    sprintf(print_buffer, "%d:", display_rows[i]); // profile point number display
    u8g2_for_adafruit_gfx.print(print_buffer);

    sprintf(print_buffer, "%ds", reflow_profile_array[selected_profile].time_steps[display_rows[i]-1]);
    rightText(print_buffer, 55, 13 + ((i + 1) * 16));

    u8g2_for_adafruit_gfx.setCursor(65, 13 + ((i + 1) * 16));
    sprintf(print_buffer, "%d°C", reflow_profile_array[selected_profile].temp_steps[display_rows[i]-1]);
    u8g2_for_adafruit_gfx.print(print_buffer);

  }

  display.display();
}

/**
 * @brief Settings menu display function
 * Displays settings name and display options
 * Allows to select scrow the menu, select and toggle through the options
 * Index correlates to setting_items array
 * @param index index for higlighted setting 
 * @param selected if highlighted setting is selected
 */
void showSettings(int index, bool selected){
  static int select_index = 0;
  static int display_rows[4] = {0, 1, 2, 3};

  if (index == display_rows[0]){ // Selection moved to top row
    select_index = 0;
  } else if(index == display_rows[1]){ // Selection moved to middle row
    select_index = 1;
  } else if (index == display_rows[2]){ // Selection moved to bottom row
    select_index = 2;
  } else if (index == display_rows[3]){ // Selection moved to bottom row
    select_index = 3;
  } else if (index > display_rows[3] && index < SETTINGS_NUM_ITEMS){ // Move select options one row down if not at end
    display_rows[0] += 1;
    display_rows[1] += 1;
    display_rows[2] += 1;
    display_rows[3] += 1;
  } else if (index < display_rows[0] && index > -1){ // Move select options one row up if not at top
    display_rows[0] -= 1;
    display_rows[1] -= 1;
    display_rows[2] -= 1;
    display_rows[3] -= 1;
  }

  display.clearDisplay();
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox2hb_tf);
  u8g2_for_adafruit_gfx.setFontMode(1); 
  for(int i = 0; i < min(4, (int)SETTINGS_NUM_ITEMS); i++){

    if(i == select_index){
      if(selected){
        display.fillRoundRect(0, (i * 16), 128, 16, 5, SSD1306_WHITE);
        u8g2_for_adafruit_gfx.setForegroundColor(INVERSE); 
      } else {
        display.drawRoundRect(0, (i * 16), 128, 16, 5, SSD1306_WHITE);
        u8g2_for_adafruit_gfx.setForegroundColor(WHITE); 
      }
    }

    u8g2_for_adafruit_gfx.setCursor(5, 13 + (i * 16));
    // Uses data stored in setting_items array to display correct setting name and selected option
    sprintf(print_buffer, "%s   %s", setting_items[display_rows[i]].display_name, 
                                      setting_items[display_rows[i]].display_options[setting_items[display_rows[i]].set_value_index]);
    u8g2_for_adafruit_gfx.print(print_buffer);
  }

  display.display();
}

/**
 * @brief Splash screen display function
 * Blocking function
 * @param display_time time in mS to show the screen for
 */
void showSplashScreen(int display_time){
  display.clearDisplay();
  display.drawBitmap(39, 0, bw_logo_50_28, 50, 28, SSD1306_INVERSE); // display BW logo

  u8g2_for_adafruit_gfx.setFont(u8g2_font_lastapprenticebold_tr);
  centeredText("PCB Reflow Hot Plate", 64, 51);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox1h_tf);
  sprintf(print_buffer, "HW: %1.1f  FW: %1.1f", HW_VERSION, FW_VERSION);
  centeredText(print_buffer, 64, 64);
  display.display();

  delay(display_time);
}

/**
 * @brief About screen display function
 * TODO: Update or create website
 * TODO: Add more info and make is scrollable
 */
void showAbout(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_lastapprenticebold_tr);
  centeredText("PCB Reflow Hot Plate", 64, 20);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_crox1hb_tf);
  centeredText("Badar's Workshop", 64, 34);
  centeredText("rhp.badar.tech", 64, 48);
  sprintf(print_buffer, "HW: %1.1f  FW: %1.1f", HW_VERSION, FW_VERSION);
  centeredText(print_buffer, 64, 64);
  display.display();
}

// ---------- Display Helper Functions ----------

/**
 * @brief Print centered text on OLED display
 * @param text text to display
 * @param x center x pixel
 * @param y center y pixel
 */
void centeredText(char text[], int x, int y){
  static int width;
  static int height;
  static int x_cursor;
  static int y_cursor;

  width = u8g2_for_adafruit_gfx.getUTF8Width(text);
  height = u8g2_for_adafruit_gfx.getFontAscent();

  x_cursor = x - round(width / 2);
  y_cursor = y - round(height / 2);

  u8g2_for_adafruit_gfx.setCursor(x_cursor, y_cursor);
  u8g2_for_adafruit_gfx.print(text);

}

/**
 * @brief Print right justified text on OLED display
 * @param text text to display
 * @param x right x pixel
 * @param y bottom y pixel
 */
void rightText(char text[], int x, int y){
  static int width;
  static int x_cursor;

  width = u8g2_for_adafruit_gfx.getUTF8Width(text);
  x_cursor = x - width;

  u8g2_for_adafruit_gfx.setCursor(x_cursor, y);
  u8g2_for_adafruit_gfx.print(text);
}

/**
 * @brief Get pixel location for Y axis ticks
 * Uses constant defined for the graph (graph_height, graph_y_origin)
 * @param percentage percentage to get the Y tick location
 * @return int tick y pixel location
 */
int getYTickPixel(int percentage){
  return ((((100 - percentage) * graph_height) / 100) - graph_y_origin); 
}