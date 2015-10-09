#ifndef SPI_H
#define SPI_H


extern uint8_t spi_sw_byte0_current_state;  //U14 switch latch 5th byte in SPI chain, first byte read
extern uint8_t spi_sw_byte0_previous_state;
extern uint8_t spi_sw_byte1_current_state;  //U9  switch latch 4th byte in SPI chain, second byte read
extern uint8_t spi_sw_byte1_previous_state;

void setup_spi(void);
void update_spi(void);
uint8_t spi_shift_byte(uint8_t);

#endif