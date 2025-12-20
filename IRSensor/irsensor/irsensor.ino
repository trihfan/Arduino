#include <LightDependentResistor.h>
#include <IRLibSendBase.h>
#include <IRLib_P02_Sony.h>

// debug
#define INPUT_FROM_SERIAL 1
#define INPUT_FROM_PI 0
#define ENABLE_SERIAL 1

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
#if ENABLE_SERIAL
  Serial.begin(9600);
  Serial.print("IR Sensor Start\n");
#endif

  pinMode(pin_input_pi, INPUT);
}

void loop() 
{
  // 1 - Read input
#if defined(INPUT_FROM_SERIAL) && defined(ENABLE_SERIAL)
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
#endif

#ifdef INPUT_FROM_PI
  // Input from pi
  int state_from_pi = digitalRead(pin_input_pi);
  Serial.print("state from pi: ");
  Serial.print(state_from_pi);
  Serial.print("\n");
  sony_ask_power_on = state_from_pi > 0;
#endif

  // 2 - Handle sony
  // Get Sony power state with photocell
  float light_intensity = photocell.getCurrentLux();
  sony_is_power_on = light_intensity > SONY_POWER_ON_LIGHT_THRESHOLD;

  // Update sony power state
  if (sony_ask_power_on != sony_is_power_on)
  {
    sender_sony.send(SONY_KEY_POWER, 15);
#ifdef ENABLE_SERIAL
    Serial.print("sony power on sent\n");
#endif
  }

  // 3 - Handle tv

  // 4 - Sleep
#ifdef ENABLE_SERIAL
  // Output state
  Serial.print("sony wanted: ");
  Serial.print(sony_ask_power_on);
  Serial.print(", current: ");
  Serial.print(sony_is_power_on);
  Serial.print(", sensor: ");
  Serial.print(light_intensity);
  Serial.print("\n");
#endif
  delay(1000);
}
