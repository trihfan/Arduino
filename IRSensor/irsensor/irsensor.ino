#include <LightDependentResistor.h>
#include <IRLibSendBase.h>
#include <IRLib_P02_Sony.h>  
#include <IRLib_HashRaw.h>

// Params
#define ENABLE_SERIAL 1
#define SLEEP_TIME 500

// Input
//#define INPUT_FROM_SERIAL
#define INPUT_FROM_PI

const int pin_input_pi = 2; // Pin used to receive state from Pi
bool input_is_on = false;

// Home cinema sony
#define HOME_CINEMA_KEY_POWER 21516
#define HOME_CINEMA_LIGHT_THRESHOLD 2
#define HOME_CINEMA_POWER_DELAY 500

LightDependentResistor photocell_home_cinema(A0, 3000, LightDependentResistor::GL5528);
IRsendSony sender_home_cinema;
bool home_cinema_is_on = false;
unsigned long home_cinema_busy_until = 0;

// Tv
#define TV_RAW_DATA_LEN 52
uint16_t tv_raw_data[TV_RAW_DATA_LEN] =
{
	3950, 3950, 500, 2000, 450, 2000, 450, 2000, 
	500, 2000, 450, 1000, 500, 1000, 450, 2050, 
	450, 1000, 500, 2000, 450, 1000, 500, 2000, 
	450, 1000, 500, 1000, 500, 1000, 450, 1050, 
	450, 1000, 500, 2000, 500, 2000, 450, 1000, 
	450, 2050, 450, 1000, 500, 2000, 450, 1000, 
	500, 2000, 500, 1000
};
#define TV_LIGHT_THRESHOLD 40
#define TV_POWER_ON_DELAY 10000
#define TV_POWER_OFF_DELAY 15000
#define TV_FALSE_ON_DELAY 10000

LightDependentResistor photocell_tv(A1, 3000, LightDependentResistor::GL5528);
IRsendRaw sender_tv;
bool tv_is_on = false;
unsigned long tv_busy_until = 0;

bool is_waiting_for_false_on = false;
unsigned long tv_changed_at = 0;

void setup() 
{
#if ENABLE_SERIAL
  Serial.begin(9600);
  Serial.print("IR Sensor Start\n");
#endif

  // set pin used to receive state from pi as input
  pinMode(pin_input_pi, INPUT);
}
  
void loop() 
{
  unsigned long time = millis();

  // 1 - Read input
#if defined(INPUT_FROM_SERIAL) && defined(ENABLE_SERIAL)
  // Allow input from serial
  if (Serial.available() > 0) 
  {
    char receivedChar = Serial.read();
    if (receivedChar == '1')
    {
      input_is_on = true;
    }
    else if (receivedChar == '0')
    {
      input_is_on = false;
    }
  }
#elif defined(INPUT_FROM_PI)
  // Input from pi
  input_is_on = digitalRead(pin_input_pi) > 0;
#endif

  // 2 - Handle home cinema
  // Get home cinema power state with photocell
  float light_intensity_home_cinema = photocell_home_cinema.getCurrentLux();
  home_cinema_is_on = light_intensity_home_cinema > HOME_CINEMA_LIGHT_THRESHOLD;
  bool code_sent = false;
  
  // Update sony power state
  if (input_is_on != home_cinema_is_on && home_cinema_busy_until < time)
  {
    sender_home_cinema.send(HOME_CINEMA_KEY_POWER, 15);
    home_cinema_busy_until = time + HOME_CINEMA_POWER_DELAY;
    code_sent = true;
#ifdef ENABLE_SERIAL
    Serial.print("home cinema power on sent\n");
#endif
  }

  // 3 - Handle tv
  // Get Sony power state with photocell
  float light_intensity_tv = photocell_tv.getCurrentLux();
  tv_is_on = light_intensity_tv > TV_LIGHT_THRESHOLD;

  // Update tv power state
  if (input_is_on != tv_is_on && !code_sent && (tv_busy_until < time))
  {
    if (!is_waiting_for_false_on && !input_is_on)
    {
      is_waiting_for_false_on = true;
      tv_changed_at = time;
    }

    if (time > (tv_changed_at + TV_FALSE_ON_DELAY))
    {
      sender_tv.send(tv_raw_data, TV_RAW_DATA_LEN, 36);
      tv_busy_until = time + (tv_is_on ? TV_POWER_OFF_DELAY : TV_POWER_ON_DELAY);
      is_waiting_for_false_on = false;
#ifdef ENABLE_SERIAL
      Serial.print("tv power on sent\n");
#endif
    }
  }
  if (is_waiting_for_false_on && input_is_on == tv_is_on && time > (tv_changed_at + TV_FALSE_ON_DELAY))
  {
    is_waiting_for_false_on = false;
  }

  // 4 - Sleep
#ifdef ENABLE_SERIAL
  // Output state
  Serial.print("input: ");
  Serial.print(input_is_on ? "on" : "off");
  Serial.print(", home cinema: ");
  Serial.print(home_cinema_is_on ? "on" : "off");
  Serial.print(", sensor: ");
  Serial.print(light_intensity_home_cinema);
  Serial.print(", tv: ");
  Serial.print(tv_is_on ? "on" : "off");
  Serial.print(", is_waiting_for_false_on: ");
  Serial.print(is_waiting_for_false_on ? "yes" : "no");
  Serial.print(", sensor: ");
  Serial.print(light_intensity_tv);
  Serial.print("\n");
#endif
  delay(SLEEP_TIME);
}
