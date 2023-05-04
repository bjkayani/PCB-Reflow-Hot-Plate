
#include "MAX6675.h"
#include <Adafruit_MLX90614.h>

const int dataPin   = 7;
const int clockPin  = 6;
const int selectPin1 = 4;
const int selectPin2 = 5;
const int thermPin = 0;

MAX6675 thermoCouple1;
MAX6675 thermoCouple2;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup()
{
  Serial.begin(115200);

  thermoCouple1.begin(clockPin, selectPin1, dataPin);
  thermoCouple2.begin(clockPin, selectPin2, dataPin);
  mlx.begin();

  thermoCouple1.setSPIspeed(4000000);
  thermoCouple2.setSPIspeed(4000000);

  mlx.writeEmissivity(1);
  Serial.print("Emissivity = "); 
  Serial.println(mlx.readEmissivity());

  pinMode(thermPin, INPUT);
  // analogSetPinAttenuation(thermPin, ADC_0db);
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

  Serial.print(",");

  Serial.print(mlx.readObjectTempC());

  Serial.print(",");

  Serial.print(analogReadMilliVolts(thermPin));

  Serial.println();

  delay(1000);
}
