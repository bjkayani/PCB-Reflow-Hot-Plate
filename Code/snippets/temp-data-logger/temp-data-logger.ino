/**
  * Temperature Data Logger
  * Author: Badar Jahangir Kayani
  * Email: badarjahangir@gmail.com
  * Date: 6/2/2023
  *
  * Reads themperature from the sensors, displays it on
  * the OLED display and sends it over the Serial port
  * 
  *   - ESP32-C3 Dev Board
  *   - 3x MAX6675 Thermocouple Module
  *   - 128x64 SSD1306 0.91" OLED Display
  *   - Optional MLX90614 IR temperature sensor 
  */

#include "MAX6675.h"
#include <Adafruit_MLX90614.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define THRM_CPL_DATA 7
#define THRM_CPL_CLK  6
#define THRM_CPL_SEL1 4
#define THRM_CPL_SEL2 5
#define THRM_CPL_SEL3 0

#define SDA_PIN 2
#define SCL_PIN 3 
#define OLED_SCREEN_ADDRESS 0x3C 

MAX6675 thermoCouple1;
MAX6675 thermoCouple2;
MAX6675 thermoCouple3;
Adafruit_MLX90614 ir = Adafruit_MLX90614();
Adafruit_SSD1306 display(128, 64, &Wire, -1);

float thrm_cpl_temp_1 = 0.0;
float thrm_cpl_temp_2 = 0.0;
float thrm_cpl_temp_3 = 0.0;

void setup()
{
  Serial.begin(115200);

  // MAX6675 thermocouple init
  thermoCouple1.begin(THRM_CPL_CLK, THRM_CPL_SEL1, THRM_CPL_DATA);
  thermoCouple2.begin(THRM_CPL_CLK, THRM_CPL_SEL2, THRM_CPL_DATA);
  thermoCouple3.begin(THRM_CPL_CLK, THRM_CPL_SEL3, THRM_CPL_DATA);

  thermoCouple1.setSPIspeed(4000000);
  thermoCouple2.setSPIspeed(4000000);
  thermoCouple3.setSPIspeed(4000000);

  // ir.begin();

  // OLED display init
  Wire.begin(SDA_PIN, SCL_PIN);
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_SCREEN_ADDRESS)) {
    Serial.println("OLED Display Init Failed");
  }
  display.clearDisplay();
}

void loop()
{
  readThrmCpl();

  printSerial();

  printDisplay();

  delay(1000);
}

void printDisplay(){
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  display.setCursor(0,0);
  display.println(thrm_cpl_temp_1);

  display.setCursor(0,20);
  display.println(thrm_cpl_temp_2);

  display.setCursor(0,40);
  display.println(thrm_cpl_temp_3);

  display.display();
}

void printSerial(){
  Serial.print(thrm_cpl_temp_1);
  Serial.print(",");
  Serial.print(thrm_cpl_temp_2);
  Serial.print(",");
  Serial.print(thrm_cpl_temp_3);
  Serial.print(",");

  Serial.println("");
}

void readThrmCpl(){
  if(!thermoCouple1.read()){
    thrm_cpl_temp_1 = thermoCouple1.getTemperature();
  } else {
    thrm_cpl_temp_1 = -999;
  }

  if(!thermoCouple2.read()){
    thrm_cpl_temp_2 = thermoCouple2.getTemperature();
  } else {
    thrm_cpl_temp_2 = -999;
  }

  if(!thermoCouple3.read()){
    thrm_cpl_temp_3 = thermoCouple3.getTemperature();
  } else {
    thrm_cpl_temp_3 = -999;
  }
}