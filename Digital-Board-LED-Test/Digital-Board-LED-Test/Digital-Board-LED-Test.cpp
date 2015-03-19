/*
 * Digital_Board_LED_Test.cpp
 *
 * Created: 2015-03-17 3:01:46 PM
 *  Author: jeff
 *
 *THIS IS JUST TO TEST ARP SYNC LED (PB7) TO MAKE SURE MONAPOLLO DIGITAL BOARD IS ALIVE AND MCU CAN BE PROGRAMMED
 */

#define F_CPU 20000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define ARP_SYNC_LED PB7




int main(void)
{
	
	//SET PORTB PIN 7 (PB7) as OUTPUT
	DDRB |= (1<<ARP_SYNC_LED);
	
	while(1)
	{
		
		PORTB |= (1<<ARP_SYNC_LED);
		_delay_ms(250);
		PORTB &= ~(1<<ARP_SYNC_LED);
		_delay_ms(250);
		
	}
}