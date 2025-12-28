#include <LightDependentResistor.h>
#include <IRLibSendBase.h>
#include <IRLib_P02_Sony.h>  
#include <IRLib_HashRaw.h>

// Params
#define ENABLE_SERIAL 1
#define SLEEP_TIME 1000

// Global
unsigned long time;

// Input
const int pin_input_pi = 2; // Pin used to receive state from Pi
bool input_is_on = false;

// Home cinema sony
#define HOME_CINEMA_KEY_POWER 21516
#define HOME_CINEMA_LIGHT_THRESHOLD 2
#define HOME_CINEMA_POWER_DELAY 3000

LightDependentResistor photocell_home_cinema(A0, 3000, LightDependentResistor::GL5528);
IRsendSony sender_home_cinema;
float light_intensity_home_cinema;
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
#define TV_LIGHT_THRESHOLD 0.1
#define TV_POWER_DELAY 3000

LightDependentResistor photocell_tv(A1, 3000, LightDependentResistor::GL5528);
IRsendRaw sender_tv;
float light_intensity_tv;
bool tv_is_on = false;
unsigned long tv_busy_until = 0;

bool code_sent = false;

// Functions
void do_tv();
void do_home_cinema();

void setup() 
{
#if ENABLE_SERIAL
  Serial.begin(9600);
  Serial.print("IR Sensor Start\n");
#endif

  // Set pin used to receive state from pi as input
  pinMode(pin_input_pi, INPUT);
}
  
void loop() 
{
  time = millis();

  // 1 - Read input
  input_is_on = digitalRead(pin_input_pi) > 0;

  // 2 - Handle home cinema
  do_home_cinema();

  // 3 - Handle tv
  if (!code_sent)
  {
    do_tv();
  }

  // 4 - Output and sleep
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
  Serial.print(", sensor: ");
  Serial.print(light_intensity_tv);
  Serial.print("\n");
#endif
  delay(SLEEP_TIME);
}

void do_tv()
{
  // Get tv power state with photocell
  light_intensity_tv = photocell_tv.getCurrentLux();
  tv_is_on = light_intensity_tv < TV_LIGHT_THRESHOLD;

  // Update tv power state
  if (input_is_on != tv_is_on && tv_busy_until < time)
  {
    tv_busy_until = time + TV_POWER_DELAY;
    sender_tv.send(tv_raw_data, TV_RAW_DATA_LEN, 36);
#ifdef ENABLE_SERIAL
    Serial.print("tv power on sent\n");
#endif
  }
}

void do_home_cinema()
{
  code_sent = false;

  // Get home cinema power state with photocell
  light_intensity_home_cinema = photocell_home_cinema.getCurrentLux();
  home_cinema_is_on = light_intensity_home_cinema > HOME_CINEMA_LIGHT_THRESHOLD;
  
  // Update home cinema power state
  if (input_is_on != home_cinema_is_on && home_cinema_busy_until < time)
  {
    sender_home_cinema.send(HOME_CINEMA_KEY_POWER, 15);
    home_cinema_busy_until = time + HOME_CINEMA_POWER_DELAY;
    code_sent = true;
#ifdef ENABLE_SERIAL
    Serial.print("home cinema power on sent\n");
#endif
  }
}
