
#include "MAX6675.h"

const int dataPin   = 7;
const int clockPin  = 6;
const int selectPin1 = 8;
const int selectPin2 = 9;

MAX6675 thermoCouple1;
MAX6675 thermoCouple2;

void setup()
{
  Serial.begin(115200);

  thermoCouple1.begin(clockPin, selectPin1, dataPin);
  thermoCouple2.begin(clockPin, selectPin2, dataPin);

  thermoCouple1.setSPIspeed(4000000);
  thermoCouple2.setSPIspeed(4000000);
}


void loop()
{
  if(!thermoCouple1.read()){
    Serial.print(thermoCouple1.getTemperature());
  }

  Serial.print(",");

  if(!thermoCouple2.read()){
    Serial.print(thermoCouple2.getTemperature());
  }

  Serial.println();

  delay(1000);
}
