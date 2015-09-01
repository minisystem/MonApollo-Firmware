/*
 * monapollo_firmware.c
 *
 * Created: 2015-06-23 5:27:02 PM
 *  Author: jeff
 */ 

#define F_CPU 20000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "spi.h"
#include "dac.h"
#include "adc.h"
#include "display.h"
#include "hardware.h"
#include "led_map.h"
#include "switch_map.h"
#include "pot_to_dac_map.h"
#include "midi.h"
#include "tune.h"
#include "utils.h"

#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
//#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;




//counter for switch scanning in main loop
uint8_t switch_timer = 0;

//MIDI gate buffer for note stealing
static uint8_t gate_buffer = 0;



void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	//value_to_display = note;
	midi_note_number = note;
	if (velocity == 0) {
		remove_note(note);
		gate_buffer--;
		if (gate_buffer == 0) PORTF &= ~(1<<GATE);
				
	} else {
		new_note(note, velocity);
		gate_buffer++; //increment gate_buffer
		//PORTF &= ~(1<<GATE); //turn gate off to re-trigger envelopes - this isn't nearly long enough
		//retriggering is a feature offered by Kenton Pro-Solo - maybe want it here, but need to decide how long to turn gate off
		//looking at gate of Pro-Solo on oscilloscope might give an idea of how long the Pro-Solo gate is released between retriggers  - checked: Pro-Solo gate-retrigger is 0.3ms
		//could implement this with timers. Will it really make a difference?
		PORTF |= (1<<GATE);
	}
	
}
void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	remove_note(note);
	gate_buffer--;
	if (gate_buffer == 0) PORTF &= ~(1<<GATE);
}

void setup_midi_usart(void)
{
    uint16_t ubbr_value = 39; //20MHz/(16*31250 BAUD) - 1
    //write ubbr_value to H and L UBBR1 registers:
    UBRR0L = (unsigned char) ubbr_value;
    UBRR0H = (unsigned char) (ubbr_value >> 8);
	
	UCSR0B = (1<<RXEN0)|(1<<TXEN0) | (1<<RXCIE0);
	//UCSR0C |= (0<<UMSEL0)|(0<<UMSEL01)|(0<<UPM01)|(0<<UPM00)|(0<<USBS0)|(0<<UCSZ02)|(1<<UCSZ01)|(1<<UCSZ00);  	
}



ISR (USART_RX_vect) { // USART receive interrupt
	 
	uint8_t inByte = UDR0;
	midi_device_input(&midi_device, 1, &inByte); 
	//calling a function in an interrupt is inefficient according to AVR C guidelines
	// so this function should maybe be inlined in main loop if inByte is made volatile	
	//***HOWEVER***, xnor-midi example code has this function being called from USART_RX_vect ISR  	
}





int main(void)
{
	//turn off JTAG so all outputs of PORTC can be used
	MCUCR = (1<<JTD);
	MCUCR = (1<<JTD);
		
	//SET PORTB PIN 7 (PB7) as OUTPUT
	DDRB |= (1<<ARP_SYNC_LED);
	
	DDRF |= (1<<GATE); //set gate as output
	//PORTF |= (1<<GATE); //turn gate on for testing
	
	DDRG |= (1<<TUNE_SELECT); //set tune select bit as output on PORTG
	//PORTG &= ~(1<<TUNE_SELECT); //set tune select bit to 0 to select VCF/VCA output for oscillator tuning
	PORTG |= (1<<TUNE_SELECT);
	
	setup_spi(); 
	
	DDRH |= (1<<POTMUX_EN0) | (1<<POTMUX_EN1); //set POTMUX_EN pins as outputs
	POT_MUX |= (1<<POTMUX_EN0) | (1<<POTMUX_EN1); //set POTMUX_EN pins HIGH (active LOW)
	//POT_MUX |= (1<<POTMUX_EN1);
	
	//set up LED display
	DDRA |= 0b11111111; //set all lines or DATA_BUS to outputs
	DATA_BUS |= 0b11111111; //set all DATA_BUS lines to HIGH (cathodes OFF)
	DDRH |= (1<<DISP_CATHODE_LATCH) | (1<<DISP_ANODE_LATCH); //set display latches to outputs
	DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH | 1<< DISP_CATHODE_LATCH); //set DISP latches to LOW (inactive)
	
	//set up switch port
	DDRF &= ~(1<<BMOD_SW); //set BMOD_SW pin as input
	
	//setup ADC
    setup_adc();		
	//setup DAC
	setup_dac();
	
	//setup MIDI
	//initialize MIDI device
	midi_device_init(&midi_device);
	//register callbacks
	midi_register_noteon_callback(&midi_device, note_on_event);
	midi_register_noteoff_callback(&midi_device, note_off_event);
	//setup MIDI USART
	setup_midi_usart();
	
	update_spi(); //initial update of SPI - will eventual be useful for picking up special power up switch holds
	
	
	//set up timer/counter0 to be clocked by T0 input
	
	//TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00) | (1<<WGM01); //clocked by external T0 pin, rising edge + clear timer on compare match
	//OCR0A = 0; //output compare register - set to number of periods to be counted. OCR0A needs to be set to (periods_to_be_counted - 1)
	//TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
	//
	//set up 16 bit timer
	
	//TCCR1B |= (1<<CS11) | (1<<CS10); //clock /64 to run at 312.5 KHz
	//TIMSK1 |= (1<<TOIE1); //enable timer1 overflow interrupt		
	

	
	////set up main timer interrupt
	////this generates the main scanning interrupt
	//TCCR2A |= (1<<CS22) | (1<<CS21); //Timer2 20MHz/256 prescaler
	//TIMSK2 |= (1<<TOIE2); //enable Timer2 overflow interrupt over flows approx. every 3ms
	

		
	sei(); //enable global interrupts
	
	////set initial pitch offset CVs
	vco1_init_cv = set_vco_init_cv(VCO1, 24079);
	vco2_init_cv = set_vco_init_cv(VCO2, 24079);
	value_to_display = compare_match_counter;//vco1_init_cv;
	
	

	while(1)
	{	
		midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
	
		update_display(value_to_display, DEC);
			
		scan_pots_and_update_control_voltages();
			
		//do SPI read/write every loops - whole section needs major update
		if (switch_timer++ == 5)
		{
			switch_timer = 0;
			update_spi();
				
		}
			
	}
}