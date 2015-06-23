_________________
 DONE:


 
DAC setup is working. Value is determined by reading 10 bit pot value
DAC MULTIPLEXING:
All pots read and appropriate DAC S&H is set

SPI LED driving and switch reading is done but only in a rudimentary fashion for testing
Leave it alone as much as possible and try to get LED 7-segment display working with
interrupt-based updates whilst keeping SPI working

Basic testing of 7-segment display is done. Cathode and anode latches and databus can
be used to light individual segments.

Main scanning interrupt (~3.3ms) implemented and SPI and LED display driving code
moved into main interrupt handler routine.

dispay_DEC function can now take any value between 0 and 9999 and display it on display
cycling through 4 places, anode 1 to 4

VCO waveform switching

 
 TO DO:
 ____________
 
*Handle decimal points on LED display
*break up code into header files
 
 
 
 COMMENTS:
 _________
 
 SPI_CLK looks funny on scope. Probing any SCK pin on 165 or 595 with scope probe
 on 1X setting causes ISW12 LED to light as if ISW12 SW had been pressed. Switching
 probe to 10X doesn't cause this problem. Must be a problem with high speed clock
 and probe impedance. SPI clock is at default - currently don't know what speed it 
 is running at.
 
 