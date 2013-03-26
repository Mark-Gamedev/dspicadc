#if defined(__dsPIC33F__)
#include "p33Fxxxx.h"
#elif defined(__PIC24H__)
#include "p24hxxxx.h"
#endif

#include <spi.h>

#define  NUM_CHS2SCAN			4		// Number of channels enabled for channel scan

/*=============================================================================
ADC INITIALIZATION FOR CHANNEL SCAN
=============================================================================*/
void initAdc1(void) {

    IPC3bits.AD1IP = 5;

    //AD1CON1bits.FORM   = 3;		// Data Output Format: Signed Fraction (Q15 format)
    AD1CON1bits.FORM = 0; // Data Output Format: Integer
    AD1CON1bits.SSRC = 2; // Sample Clock Source: GP Timer starts conversion
    AD1CON1bits.ASAM = 1; // ADC Sample Control: Sampling begins immediately after conversion
    AD1CON1bits.AD12B = 0; // 10-bit ADC operation
    AD1CON1bits.SIMSAM = 1;

    AD1CON2bits.CSCNA = 1; // Scan Input Selections for CH0+ during Sample A bit
    AD1CON2bits.CHPS = 0; // Converts CH0

    AD1CON3bits.ADRC = 0; // ADC Clock is derived from Systems Clock
    AD1CON3bits.ADCS = 63; // ADC Conversion Clock Tad=Tcy*(ADCS+1)= (1/40M)*64 = 1.6us (625Khz)
    // ADC Conversion Time for 10-bit Tc=12*Tab = 19.2us

    AD1CON2bits.SMPI = (NUM_CHS2SCAN - 1); // 4 ADC Channel is scanned

    //AD1CSSH/AD1CSSL: A/D Input Scan Selection Register
    AD1CSSLbits.CSS0 = 1; // Enable AN0 for channel scan
    AD1CSSLbits.CSS1 = 1; // Enable AN1 for channel scan
    AD1CSSLbits.CSS4 = 1; // Enable AN4 for channel scan
    AD1CSSLbits.CSS5 = 1; // Enable AN5 for channel scan

    //AD1PCFGH/AD1PCFGL: Port Configuration Register
    AD1PCFGL = 0xFFFF;
    AD1PCFGLbits.PCFG0 = 0; // AN0 as Analog Input
    AD1PCFGLbits.PCFG1 = 0; // AN1 as Analog Input
    AD1PCFGLbits.PCFG4 = 0; // AN4 as Analog Input
    AD1PCFGLbits.PCFG5 = 0; // AN5 as Analog Input


    IFS0bits.AD1IF = 0; // Clear the A/D interrupt flag bit
    IEC0bits.AD1IE = 1; // Enable A/D interrupt
    AD1CON1bits.ADON = 1; // Turn on the A/D converter

}

void disableADCInt(){
    IPC3bits.AD1IP = 0;
}

void enableADCInt(){
    IPC3bits.AD1IP = 5;
}
/*=============================================================================
Timer 3 is setup to time-out every 125 microseconds (8Khz Rate). As a result, the module
will stop sampling and trigger a conversion on every Timer3 time-out, i.e., Ts=125us.
=============================================================================*/
void initTmr3() {
    TMR3 = 0x0000;
    PR3 = 49;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 0;

    //Start Timer 3
    T3CONbits.TON = 1;

}

/*=============================================================================
ADC INTERRUPT SERVICE ROUTINE
=============================================================================*/

int  scanCounter=0;
int  sampleCounter=0;

//test threshold
int triggerSampleNumber[3];
#define THRESHOLD 800
void testThreshold(int value){
    if(value > THRESHOLD){
        triggerSampleNumber[scanCounter] = sampleCounter;
    }

    if(triggerSampleNumber[0] != 0 && triggerSampleNumber[1] != 0){
        int diff = triggerSampleNumber[0] - triggerSampleNumber[1];
        triggerSampleNumber[0] = 0;
        triggerSampleNumber[1] = 0;
    }
}

void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void) {
    unsigned int value = ADC1BUF0;
    // each spi transfer is 16 bits
    // 0sss ccdd dddd dddd
    // s -- sequence number/sampleCounter
    // c -- channel number
    // d -- data

    if(scanCounter!=3){
        int data;
        data = (sampleCounter  << 12) | scanCounter << 10 | value;
        addToSPIBuffer(data);
    }

    scanCounter++;
    if (scanCounter == NUM_CHS2SCAN) {
        scanCounter = 0;
        sampleCounter++;
    }

    IFS0bits.AD1IF = 0; // Clear the ADC1 Interrupt Flag
}
