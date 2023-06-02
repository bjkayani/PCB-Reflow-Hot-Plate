/**
 * PCB Reflow Hot Plate
 * led.ino 
 * Contains functions for the RGB led
 */


void ledHeatDisplay(){
    pixels.setPixelColor(0, pixels.Color(255, 0, 0));
    pixels.setBrightness(255);
    pixels.show();            // Turn OFF all pixels ASAP
  
}

void ledLoop(){

}