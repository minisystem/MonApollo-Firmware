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
#include <avr/eeprom.h>
#include "assigner.h"
#include "spi.h"
#include "dac.h"
#include "adc.h"
#include "display.h"
#include "hardware.h"
#include "led_map.h"
#include "switch_map.h"
#include "pot_to_dac_map.h"
#include "midi.h"
#include "synth.h"
#include "tune.h"
#include "utils.h"
#include "clock.h"

#include "xnormidi-develop/midi.h"
#include "xnormidi-develop/midi_device.h"
//#include "xnormidi-develop/bytequeue/bytequeue.h" //this is required for MIDI sending

MidiDevice midi_device;


//uint16_t EEMEM test_word_eeprom;
//uint16_t test_word = 1234;
//
//counter for switch scanning in main loop
uint8_t switch_timer = 0;

//MIDI gate buffer for note stealing
static uint8_t gate_buffer = 0;




void note_on_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	
	if ((current_patch.byte_2 >> LFO_KEY_SYNC) & 1) {
		
		PORTB |= (1<< LFO_RESET);
		_delay_us(1); //what is minimum pulse width required for LFO reset?
		PORTB &= ~(1<< LFO_RESET);
			
	}		
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
	//PORTB &= ~(1<< LFO_RESET);
	
}
void note_off_event(MidiDevice * device, uint8_t status, uint8_t note, uint8_t velocity) {
	remove_note(note);
	gate_buffer--;
	if (gate_buffer == 0) PORTF &= ~(1<<GATE);
}

void real_time_event(MidiDevice * device, uint8_t real_time_byte) {
	//PORTB ^= (1<<ARP_SYNC_LED);
	//if (~(current_patch.byte_2 & (1<<LFO_KEY_SYNC) >> 1)) return; //if not in a sync mode, then retrun
	switch (real_time_byte) {
		
		case MIDI_CLOCK:
		
			if (++midi_clock.ppqn_counter == midi_clock.divider) {
				PORTB |= (1<< LFO_RESET);
				_delay_us(1); //what is minimum pulse width required for LFO reset?
				
				PORTB ^= (1<<ARP_SYNC_LED);
				//register clock event - this will do something  - reset LFO or initiate LFO
				midi_clock.ppqn_counter = 0; //reset MIDI ppqn clock	
				PORTB &= ~(1<< LFO_RESET); //turn off LFO reset pin
			}
			
			break;
			
		case MIDI_START:
			
			midi_clock.ppqn_counter = 0;
			break;
			
		case MIDI_STOP:
		
			break;		
		
	}
	
	
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
	//PORTB ^= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED 
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
	
	DDRB |= (1<< LFO_RESET); //set LFO reset as output. This affects LFO rate and TRI balance - needed to trim both rate and balance.
	//PORTB |= (1<< LFO_RESET);
	
	
	DDRF |= (1<<GATE); //set gate as output
	//PORTF |= (1<<GATE); //turn gate on for testing
	
	DDRG |= (1<<TUNE_SELECT); //set tune select bit as output on PORTG
	PORTG &= ~(1<<TUNE_SELECT); //set tune select bit to 0 to select VCF/VCA output for oscillator tuning
	//PORTG |= (1<<TUNE_SELECT);
	
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
	
	//set up LFO DEMUX LATCH
	DDRJ |= (1<<LFO_SW_LATCH); //set LFO_SW_LATCH pin as output
	DATA_BUS = LFO_TRI_ADDR;
	LFO_LATCH_PORT |= (1<<LFO_SW_LATCH);
	LFO_LATCH_PORT &= ~(1<<LFO_SW_LATCH);
	DATA_BUS = 0;
	current_patch.byte_2 = (1<<LFO_TRI);
	
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
	midi_register_realtime_callback(&midi_device, real_time_event);
	//setup MIDI USART
	setup_midi_usart();
	
	update_spi(); //initial update of SPI - will eventual be useful for picking up special power up switch holds
	
	
	current_patch.number = 1;
		
	sei(); //enable global interrupts
	
	////set initial pitch offset CVs
	//vco1_init_cv = set_vco_init_cv(VCO1, 24079);
	//vco2_init_cv = set_vco_init_cv(VCO2, 24079);
	//value_to_display = compare_match_counter;//vco1_init_cv;
	

	
	//eeprom_update_word((uint16_t*)109, test_word);
	//value_to_display = eeprom_read_word((uint16_t*)109);
	
	load_tuning_tables();
	//value_to_display = vco1_init_cv;
	//set_one_volt_per_octave(); //overwrite tuning tables with 1V/octave data for calibration purposes
	//set initial switch states
	//switch_states.byte0 = (1<<VCO1_PULSE_SW) | (1<<VCO2_PULSE_SW);
	//current_patch.byte_4 = (1<<VCO1_32F) | (1<<VCO2_32F);
	load_patch(1);
	
	setup_system_clock();
	//update_clock_speed(244);
	system_clock.divider = 24;
	

	while(1)
	{	
		
		
		midi_device_process(&midi_device); //this needs to be called 'frequently' in order for MIDI to work
		//value_to_display = vco1_init_cv;
		//PORTB |= (1<<ARP_SYNC_LED); //toggle arp VCO_SYNC_LATCH_BIT LED
		update_display(value_to_display, DEC); //maybe move this into switch_timer loop. Probably doesn't need to be updated this frequently
		//PORTB &= ~(1<<ARP_SYNC_LED);
		 	
		//scan_pots_and_update_control_voltages();
		scan_pots();
		update_control_voltages();
			
		//do SPI read/write every 5 loops - whole section needs major update
		if (switch_timer++ == 5)
		{
			switch_timer = 0;
			switch_press = 0; //reset global switch press flag
			//read switches directly connected to MCU	
			switch_states.byte2 ^= read_switch_port(); //toggle switch states
			switch_states.byte2 |= (current_patch.mode == MANUAL) << PROG_MANUAL_SW; //if MANUAL then don't toggle switch
			//switch_states.byte2 |= (current_patch.mode == WRITE) << PROG_WRITE_SW; //if WRITE then don't toggle switch
			update_spi();
			if (switch_press) { 
				update_patch();
				if (current_patch.mode == MEMORY) current_patch.mode = EDIT; //change mode to EDIT if non-program switch is detected
			}				
			update_patch_programmer();	
			
		}
		
			
	}
}