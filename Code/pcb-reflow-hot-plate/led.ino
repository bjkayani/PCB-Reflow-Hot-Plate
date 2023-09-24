/**
 * PCB Reflow Hot Plate
 * led.ino 
 * Contains functions for the RGB led
 */

/**
 * @brief Set led color based on temperature Red-Green
 * @param temp Temperature to map into color
 * @param brightness LED brightness
 */
void ledHeatDisplay(int temp, int brightness){
    if(temp < MIN_TEMP){
        temp = MIN_TEMP;
    } else if (temp > MAX_TEMP){
        temp = MAX_TEMP;
    }
    
    int color_map = map(temp, MIN_TEMP, MAX_TEMP, 0, 255);
    pixels.setPixelColor(0, pixels.Color(color_map, 255 - color_map, 0));
    pixels.setBrightness(brightness);
    pixels.show(); 
}

/**
 * @brief Set static LED color
 * @param color Color code to set based on color_t enum
 * @param brightness LED brightness
 */
void ledSetColor(int color, int brightness){
    switch(color){
        case RED:
            pixels.setPixelColor(0, pixels.Color(255, 0, 0));
            break;
        case GREEN:
            pixels.setPixelColor(0, pixels.Color(0, 255, 0));
            break;
        case BLUE:
            pixels.setPixelColor(0, pixels.Color(0, 0, 255));
            break;
        default:
            pixels.setPixelColor(0, pixels.Color(0, 0, 0));
            break;
    }

    pixels.setBrightness(brightness);
    pixels.show();  
}