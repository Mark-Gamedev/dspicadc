#if defined(__dsPIC33F__)
#include "p33Fxxxx.h"
#elif defined(__PIC24H__)
#include "p24fxxxx.h"
#endif

#include <stdlib.h>
#include "circularBuffer.h"

//Macros for Configuration Fuse Registers:
//Invoke macros to set up  device configuration fuse registers.
//The fuses will select the oscillator source, power-up timers, watch-dog
//timers etc. The macros are defined within the device
//header files. The configuration fuse registers reside in Flash memory.

// Internal FRC Oscillator
_FOSCSEL(FNOSC_FRC); // FRC Oscillator
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF & POSCMD_NONE);
// Clock Switching is enabled and Fail Safe Clock Monitor is disabled
// OSC2 Pin Function: OSC2 is Clock Output
// Primary Oscillator Mode: Disabled

_FWDT(FWDTEN_OFF); // Watchdog Timer Enabled/disabled by user software
// (LPRC can be disabled by clearing SWDTEN bit in RCON register

void switchToPLL() {
    // Configure Oscillator to operate the device at 40Mhz
    // Fosc= Fin*M/(N1*N2), Fcy=Fosc/2
    // Fosc= 8M*40/(2*2)=80Mhz for 8M input clock
    PLLFBD = 30;              // M=40
    CLKDIVbits.PLLPOST = 0;   // N1=2
    CLKDIVbits.PLLPRE = 0;    // N2=2
    OSCTUN = 0;               // Tune FRC oscillator, if FRC is used

    // Disable Watch Dog Timer
    RCONbits.SWDTEN = 0;

    // clock switch to incorporate PLL
    __builtin_write_OSCCONH(0x03); // Initiate Clock Switch to Primary
    // Oscillator with PLL (NOSC=0b011)
    __builtin_write_OSCCONL(0x01); // Start clock switching
    while (OSCCONbits.COSC != 0b011); // Wait for Clock switch to occur

    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1);
}

int main(void) {
    switchToPLL();

    cbInit();

    // Peripheral Initialisation
    configSpiPins();
    initSPI();  // Initialize SPI
    initAdc1(); // Initialize ADC
    initTmr3(); // Initialise TIMER 3

    // Background Loop
    while (1) {
    }
}
