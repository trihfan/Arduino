// TrihfanDelay:
//
// -----------------------------------------------------------------------
// Original code:
// Licensed under a Creative Commons Attribution 3.0 Unported License.
// Based on rcarduino.blogspot.com previous work.
// www.electrosmash.com/pedalshield

/*  Max delay: 605ms
 *
 */
// Include
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 | U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST); // Fast I2C / TWI
#include "SplashScreen.h"
#include "DelayBuffer.h"

// Definitions
#define RAM_SIZE 640000
#define BUFFER_SIZE RAM_SIZE / 2 
#define MAX_INDEX BUFFER_SIZE / 16

// Input/Output
int input_adc0, input_adc1;     // Variables for 2 ADCs input values (ADC0, ADC1)
int output_adc0, output_adc1;   // Variables for 2 DACs output values (DAC0, DAC1)
int duration_pot = -1;          // First pot, duration
int feedback_pot = -1;          // Second pot, feedback
int effect_level_pot = -1;      // Third pot, effect level
int led = 3;                    // The led, 
int footswitch = 7;             // Footswitch, enable or disable the effect
int toggle = 2; 

// Variables
DelayBuffer<BUFFER_SIZE> buffer_adc0, buffer_adc1;    // Buffers for adc0 and adc1
size_t current_index = 0;                                       // The current index of the buffers
size_t max_index = MAX_INDEX;                                   // The maximum index of the buffers

int duration_percent = 0;       // The percentage of the duration pot
int effect_level_percent = 0;   // The percentage of the effect level pot
int feedback_percent = 0;       // The percentage of the feedback pot

bool led_on = true;             // Current led state

// Setup arduino
void setup()
{
    // Setup screen
    u8g.setColorIndex(1); // pixel on
    u8g.firstPage();
    do
    {
        u8g.drawBitmapP(0, 0, 16, 64, splash_screen);
    } 
    while (u8g.nextPage());

    // Turn on the timer clock in the power management controller
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk(ID_TC4);

    // We want wavesel 01 with RC
    TC_Configure(TC1, 1, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK2);

    // set_frequency();
    TC_SetRC(TC1, 1, 238);
    TC_Start(TC1, 1);

    // Enable timer interrupts on the timer
    TC1->TC_CHANNEL[1].TC_IER = TC_IER_CPCS;
    TC1->TC_CHANNEL[1].TC_IDR = ~TC_IER_CPCS;

    // Enable the interrupt in the nested vector interrupt controller
    // TC4_IRQn where 4 is the timer number * timer channels (3) + the channel number
    // (=(1*3)+1) for timer1 channel1
    NVIC_EnableIRQ(TC4_IRQn);

    // ADC Configuration
    ADC->ADC_MR |= 0x80;    // DAC in free running mode.
    ADC->ADC_CR = 2;        // Starts ADC conversion.
    ADC->ADC_CHER = 0x1CC0; // Enable ADC channels 0,1,8,9 and 10

    // DAC Configuration
    analogWrite(DAC0, 0); // Enables DAC0
    analogWrite(DAC1, 0); // Enables DAC0

    // PedalSHIELD pin configuration
    pinMode(led, OUTPUT);
    pinMode(footswitch, INPUT_PULLUP);
    pinMode(toggle, INPUT_PULLUP); 
}

// Main loop
void loop()
{
    // wait for ADC 0, 1, 8, 9, 10 conversion complete
    while ((ADC->ADC_ISR & 0x1CC0) != 0x1CC0);  

    // Read the ADCs
    input_adc0 = ADC->ADC_CDR[7];           // read data from ADC0
    input_adc1 = ADC->ADC_CDR[6];           // read data from ADC1
    duration_pot = ADC->ADC_CDR[10];        // read data from ADC8
    feedback_pot = ADC->ADC_CDR[11];        // read data from ADC9
    effect_level_pot = ADC->ADC_CDR[12];    // read data from ADC10

    // Check change
    int duration_percent_new = duration_pot * 100 / 4096;
    int effect_level_percent_new = effect_level_pot * 100  / 4096;
    int feedback_percent_new = feedback_pot * 100  / 4096;

    bool redraw = duration_percent != duration_percent_new ||
                  effect_level_percent != effect_level_percent_new || 
                  feedback_percent != feedback_percent_new;

    // Handle led    
    if (digitalRead(footswitch))
    {
        digitalWrite(led, HIGH);
    }
    else
    {
        digitalWrite(led, LOW);
    }

    // Draw screen if a value has changed
    if (redraw)
    {
        duration_percent = duration_percent_new;
        effect_level_percent = effect_level_percent_new;
        feedback_percent = feedback_percent_new;

        // picture loop
        u8g.firstPage();  
        do {
           draw();
        } 
        while(u8g.nextPage()); 
    }
}

// Timer interrupt
void TC4_Handler()
{
    // Compute the new value depending on feedback
    // TODO
    
    // Store current readings
    buffer_adc0.read(current_index) = input_adc0;
    buffer_adc1.read(current_index) = input_adc1;

    // Adjust max_index based in duration pot position.
    max_index = map(duration_pot >> 2, 0, 1023, 1, MAX_INDEX);

    // Increse/reset delay counter.
    current_index++;
    if (current_index >= max_index)
    {
        current_index = 0;
    }

    buffer_adc0.write(current_index);
    buffer_adc1.write(current_index);

    // Add volume feature
    output_adc0 = map(output_adc0, 0, 4095, 1, effect_level_pot);
    output_adc1 = map(output_adc1, 0, 4095, 1, effect_level_pot);

    // Write the DACs
    dacc_set_channel_selection(DACC_INTERFACE, 0);        //select DAC channel 0
    dacc_write_conversion_data(DACC_INTERFACE, output_adc0); //write on DAC
    dacc_set_channel_selection(DACC_INTERFACE, 1);        //select DAC channel 1
    dacc_write_conversion_data(DACC_INTERFACE, output_adc1); //write on DAC

    // Clear status allowing the interrupt to be fired again.
    TC_GetStatus(TC1, 1);
}

// Draw screen
void draw()
{
    u8g.setFont(u8g_font_6x10);

    // Duration
    u8g.setPrintPos(6, 22); 
    float duration_value = (duration_percent / 100.f) * (MAX_INDEX / 44100.f);
    u8g.print("Delay: "); u8g.print(int(duration_value * 1000)); u8g.print("ms");
    u8g.drawLine(4, 27, 4 + int(124 * (duration_percent / 100.f)), 27);
    u8g.drawLine(4, 28, 124, 28);
    u8g.drawPixel(4, 26);
    u8g.drawPixel(124, 26);
    u8g.drawPixel(124, 4274);

    // Effect level   
    u8g.setPrintPos(6, 39);
    u8g.print("Effect Level: "); u8g.print(effect_level_percent); u8g.print('%');
    u8g.drawLine(4, 44, 4 + int(124 * (effect_level_percent / 100.f)), 44);
    u8g.drawLine(4, 45, 124, 45);
    u8g.drawPixel(4, 43);
    u8g.drawPixel(124, 43);
    u8g.drawPixel(124, 44);

    // Feedback
    /*u8g.setPrintPos(6, 56);
    u8g.print("Feedback: "); u8g.print(feedback_percent); u8g.print('%');
    u8g.drawLine(4, 61, 4 + feedback_percent, 61);
    u8g.drawLine(4, 62, 124, 62);
    u8g.drawPixel(4, 60);
    u8g.drawPixel(124, 60);
    u8g.drawPixel(124, 61);*/
}