#include <LightDependentResistor.h>
#include <IRLibSendBase.h>
#include <IRLib_P02_Sony.h>

#define SONY_KEY_POWER 21516
#define SONY_POWER_ON_TIME 3000
#define SONY_POWER_ON_LIGHT_THRESHOLD 4

LightDependentResistor photocell(A0, 3000, LightDependentResistor::GL5528);

IRsendSony sender_sony;

bool sony_is_power_on = false;
bool sony_ask_power_on = false;

// Communication with Pi
const int pin_input_pi = 2;

void setup() 
{
  Serial.begin(9600);
  Serial.print("IR Sensor Start\n");

  pinMode(pin_input_pi, INPUT);
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

  // Input from pi
  int etat = digitalRead(pin_input_pi);
  Serial.print(etat);
  Serial.print("\n");

  // Get Sony On Off state with photocell
  float light_intensity = photocell.getCurrentLux();
  sony_is_power_on = light_intensity > SONY_POWER_ON_LIGHT_THRESHOLD;

  //
  if (sony_ask_power_on != sony_is_power_on)
  {
    sender_sony.send(SONY_KEY_POWER, 15);
    //Serial.print("sony power on sent\n");
  }

  //
  /*Serial.print("sony wanted: ");
  Serial.print(sony_ask_power_on);
  Serial.print(", current: ");
  Serial.print(sony_is_power_on);
  Serial.print(", sensor: ");
  Serial.print(light_intensity);
  Serial.print("\n");*/
  delay(1000);
}
