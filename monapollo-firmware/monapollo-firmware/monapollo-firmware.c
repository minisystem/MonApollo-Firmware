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

#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
//#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;

volatile uint16_t value_to_display = 79; //global to hold display value

//counter for switch scanning
volatile uint8_t switch_timer = 0;

volatile uint8_t place = 0; //digit place for LED display

void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	value_to_display = note;
	
	if (velocity == 0) {
		
		PORTF &= ~(1<<GATE);
	} else {
		PORTF |= (1<<GATE);
	}
	
}
void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	PORTF &= ~(1<<GATE);
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


volatile uint8_t digit[] = {
	ONES,
	TENS,
	HUNDS,
	THOUS,
};



ISR (USART_RX_vect) { // USART receive interrupt
	 
	uint8_t inByte = UDR0;
	midi_device_input(&midi_device, 1, &inByte);	
	  	
}

ISR (TIMER2_OVF_vect) { //main scanning interrupt handler
	
	display_dec(value_to_display, digit[place]);
	
	scan_pots_and_update_control_voltages();		

		
	//do SPI read/write every 5 interrupts (16.5 ms)
	if (switch_timer++ == 5)
	{
		switch_timer = 0;
		update_spi();	
			  	
	}
		
	//increment digit display place
	if (place++ == 3) //post increment
	{
		place = 0;
	}
	

	
}	

ISR (TIMER0_COMP_vect) {
	
	PORTB ^= (1<<ARP_SYNC_LED); //toggle arp sync LED
	TCNT0 = 0; //reset timer
	//value_to_display = TCNT0;
	
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
	DDRF &= ~(1<<ISW8_SW); //set ISW8_SW pin as input
	
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
	
	//set up main timer interrupt
	//this generates the main scanning interrupt
	TCCR2A |= (1<<CS22) | (1<<CS21); //Timer2 20MHz/256 prescaler
	TIMSK2 |= (1<<TOIE2); //enable Timer2 overflow interrupt over flows approx. every 3ms
	
	//set up timer/counter0 to be clocked by T0 input
	TCCR0A |= (1<<CS02) | (1<<CS01) | (1<<CS00); //clocked by external T0 pin, rising edge
	OCR0A = 1; //output compare register - set to number of periods to be counted.
	TIMSK0 |= (1<<OCIE0A); //enable output compare match A interrupt
		
	sei(); //enable global interrupts

	while(1)
	{	
		midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
		//PORTB |= (1<<ARP_SYNC_LED);
		//value_to_display = TCNT0;
	}
}