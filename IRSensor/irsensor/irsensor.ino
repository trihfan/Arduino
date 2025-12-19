#include <LightDependentResistor.h>
#include <IRLibAll.h>
#include <IRLib_HashRaw.h>

#define SONY_KEY_POWER 21516
#define SONY_POWER_ON_TIME 3000

LightDependentResistor photocell(A0, 3000, LightDependentResistor::GL5528);

bool sony_is_power_on = false;
bool sony_ask_power_on = false;

char buffer[150];

void setup() 
{
  Serial.begin(9600);
  Serial.print("IR Sensor Start\n");
}

void loop() 
{
  // Allow input from serial
  if (Serial.available() > 0) 
  {
    char receivedChar = Serial.read();
    if (receivedChar == '1')
    {
      sony_ask_power_on = true;
    }
    else if (receivedChar == '0')
    {
      sony_ask_power_on = false;
    }
  }

  // Get Sony On Off state with photocell
  float light_intensity = photocell.getCurrentLux();

  //
  sprintf(buffer, "%d -> %d\n", sony_ask_power_on, sony_is_power_on);
  Serial.print(buffer);
  delay(1000);
}
