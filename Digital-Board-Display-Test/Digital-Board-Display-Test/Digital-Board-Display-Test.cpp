/*
 * Digital_Board_Display_Test.cpp
 *
 * Created: 2015-03-23 10:47:57 PM
 *  Author: jeff
 
 *LED DISPLAY TEST
 ==========================================================
 
 DONE:
 ==============================================================================
 
SPI LED driving and switch reading is done but only in a rudimentary fashion for testing
Leave it alone as much as possible and try to get LED 7-segment display working with
interrupt-based updates whilst keeping SPI working

Basic testing of 7-segment display is done. Cathode and anode latches and databus can
be used to light individual segments
 
 TO DO:
 ==============================================================================
 
implement scanning interrupt routine to light display one digit at a time. Have a look
at DAC Development Board code.
 
 ==============================================================================
 
 COMMENTS:
 ==============================================================================
 
 SPI_CLK looks funny on scope. Probing any SCK pin on 165 or 595 with scope probe
 on 1X setting causes ISW12 LED to light as if ISW12 SW had been pressed. Switching
 probe to 10X doesn't cause this problem. Must be a problem with high speed clock
 and probe impedance. SPI clock is at default - currently don't know what speed it 
 is running at.
 
 ==============================================================================
 */

#define F_CPU 20000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//ARP_SYNC LED driven directly from AVR
#define ARP_SYNC_LED	PB7

//SPI pins
#define SPI_DATA_OUT	(1<<PB2)
#define SPI_DATA_IN		PB3
#define SPI_CLK			(1<<PB1)

//SPI EN pin on PORTJ
#define SPI_EN			(1<<PJ2)

//LED latch pin on PORTJ
#define LED_LATCH		(1<<PJ3)

//define PORTS
#define SPI_PORT PORTB
#define SPI_LATCH_PORT PORTJ
#define DATA_BUS PORTA
#define DISPLAY_PORT PORTH

//define LED bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_LED			0b00000100
#define ISW11_LED			0b10000000
#define ISW8_LED			0b10000000
#define ISW4_LED			0b00000010

//define switch bits - NEED TO CHANGE THESE TO BIT POSITIONS 0-7
#define ISW12_SW			0b00100000
#define ISW13_SW			0b01000000
#define ISW4_SW				0b10000000

//SPI switch latch
#define SPI_SW_LATCH		(1<<PB5)

//LED 7-segment display latches
#define DISP_ANODE_LATCH	PH5
#define DISP_CATHODE_LATCH	PH4

int main(void)
{
	
	//SET PORTB PIN 7 (PB7) as OUTPUT
	DDRB |= (1<<ARP_SYNC_LED);
	
	//SET SPI_DATA_OUT and SPI_CLK and SPI_SW_LATCH pins as outputs
	//also set Slave Select (PB0) as output just to ensure it doesn't interfere with SPI communication (currently floating)
	//ACTUALLY, Slave Select ***MUST*** be set as output. Leaving it floating without setting its data direction bit breaks SPI!
	DDRB |= (SPI_DATA_OUT | SPI_CLK | SPI_SW_LATCH |(1<<PB0));
	
	//SET SPI_EN and LED_LATCH pins as outputs
	DDRJ |= (SPI_EN | LED_LATCH);
	
	//SET SPI_DATA_OUT and SPI_CLK and SPI_SW_LATCH outputs LOW
	SPI_PORT &= ~(SPI_DATA_OUT | SPI_CLK | SPI_SW_LATCH);
	
	//SET SPI_EN LOW (active) and LED_LATCH LOW (active)
	SPI_LATCH_PORT &= ~(SPI_EN | LED_LATCH);
	
	//SET UP SPI
	SPCR = (1<<SPE) | (1<<MSTR); //Start SPI as MASTER
	
	//Pull LED_LATCH LOW
	SPI_LATCH_PORT &= ~LED_LATCH;
	
	////SHIFT DATA IN TO LIGHT ISW12
	//SPDR = 0b11111111; 
	////Wait for SPI shift to complete
	//while (!(SPSR & (1<<SPIF)));
	
	//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
	
	SPI_LATCH_PORT &= ~LED_LATCH;
	SPI_LATCH_PORT |= LED_LATCH;
	
	uint8_t ISW12_SW_ON = 0; //flag for ISW12 switch
	uint8_t ISW13_SW_ON = 0; //flag for ISW13 switch
	uint8_t ISW4_SW_ON = 0;  //flag for ISW4 switch
	
	//set up LED display
	DDRA |= 0b11111111; //set all lines or DATA_BUS to outputs
	DATA_BUS |= 0b11111111; //set all DATA_BUS lines to HIGH (cathodes OFF)
	DDRH |= (1<<DISP_CATHODE_LATCH) | (1<<DISP_ANODE_LATCH); //set display latches to outputs
	DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH | 1<< DISP_CATHODE_LATCH); //set DISP latches to LOW (inactive)
	
	while(1)
	{
		
		PORTB |= (1<<ARP_SYNC_LED);
		
		//SET SPI_SW_LATCH HI - this latches switch data into 74XX165 shift registers for SPI transfer
		SPI_PORT |= SPI_SW_LATCH;		
		
		//SHIFT 5th BYTE
		SPDR =  ISW4_SW_ON << 1 | ISW8_LED; //ISW8_LED is MSB on 74XX595 U16
		while (!(SPSR & (1<<SPIF)));
		
		//Now read SPDR for switch data shifted in from 74XX165 U14		
		if (SPDR >> 7 & 1) //check if ISW4_SW bit is set (MSB on U14)
		{
			ISW4_SW_ON = 1;
		}
		else
		{
			ISW4_SW_ON = 0;
		}		
		//SHIFT 4th BYTE
		SPDR = 0; //no LEDs connected in current test set up
		while (!(SPSR & (1<<SPIF)));
		//Now read SPDR for switch data shifted in from 74XX165 (U9)
		//check if ISW12_SW bit is set
		if (SPDR >> 5 & 1)
		{
			ISW12_SW_ON = 1;
		}
		else
		{
			ISW12_SW_ON = 0;
		}
		//check if ISW13_SW bit is set
		if (SPDR >> 6 & 1)
		{
			ISW13_SW_ON = 1;
		}
		else
		{
			ISW13_SW_ON = 0;
		}
		
		//SHIFT 3th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));				

		//SHIFT 2th BYTE
		SPDR = 0;
		while (!(SPSR & (1<<SPIF)));
					
		//SHIFT 1st BYTE
		//SPDR = (ISW12_SW_ON << 2) | ISW11_LED; //TURN ON ISW12 (if ISW12_SW is ON) and ISW11 LEDs, both on 74XX595 U8, first shift register in chain
		SPDR = (ISW12_SW_ON <<2) | (ISW13_SW_ON << 7); //turn on ISW12 if ISW12_SW is ON, turn ISW11 (MSB of first shift register chain) if ISW13_SW is ON
		//Wait for SPI shift to complete
		while (!(SPSR & (1<<SPIF)));
		
		//Toggle LED_LATCH to shift data to 74HC595 shift register outputs
		
		SPI_LATCH_PORT &= ~LED_LATCH;
		SPI_LATCH_PORT |= LED_LATCH;
		
		//clear SPI_SW_LATCH
		SPI_PORT &= ~SPI_SW_LATCH;
		
		//LIGHT SOME SEGMENTS OF 7-SEG LED DISPLAY
		DATA_BUS = 0b1111; //set bits for anode
		//latch data to anode lines
		DISPLAY_PORT |= (1<<DISP_ANODE_LATCH);	
		DISPLAY_PORT &= ~(1<<DISP_ANODE_LATCH);
		
		DATA_BUS = 0b00000001; //set bits for cathode (current sinks, active LOW)
		//toggle data to cathode lines
		DISPLAY_PORT |= (1<<DISP_CATHODE_LATCH);
		DISPLAY_PORT &= ~(1<<DISP_CATHODE_LATCH);
		
		
		
	}
}