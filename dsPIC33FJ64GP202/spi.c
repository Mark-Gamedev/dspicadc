/* 
 * File:   myspi.c
 * Author: mark
 *
 * Created on March 22, 2013, 1:46 PM
 */

#include "p33Fxxxx.h"
#include <spi.h>

#define TXBUFSIZE 3
static unsigned int txBuffer[TXBUFSIZE];
static int sendPos, fillPos, startSend;

void configSpiPins(){
    //remap pins

    TRISB = 0xFFFF;
    TRISBbits.TRISB6 = 0;     // RB6 as output

    RPINR20bits.SDI1R = 9;    // SPI1 data input tie to RP14
    RPINR20bits.SCK1R = 8;    // SPI1 clock input tie to RP13
    RPINR21bits.SS1R = 7;     // SPI1 slave select tie to RP12
    RPOR3bits.RP6R = 0b00111; // RP11 tie to SPI data out
}

void initSPI(void) {

    IPC2bits.SPI1EIP = 7;

    /* The following code shows the SPI register configuration for Slave mode */
    SPI1BUF = 0;
    IFS0bits.SPI1IF = 0; // Clear the Interrupt Flag
    IEC0bits.SPI1IE = 0; // Disable the Interrupt
    // SPI1CON1 Register Settings
    SPI1CON1bits.DISSCK = 0; // Internal Serial Clock is Enabled
    SPI1CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI1CON1bits.MODE16 = 1; // Communication is word-wide (16 bits)
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data
    // output time
    SPI1CON1bits.CKE = 1; // Serial output data changes on transition
    // from Idle clock state to active clock state
    SPI1CON1bits.CKP = 0; // Idle state for clock is a low level; active
    // state is a high level
    SPI1CON1bits.MSTEN = 0; // Master mode Disabled
    SPI1CON1bits.SSEN = 1;
    SPI1STATbits.SPIROV = 0; // No Receive Overflow has occurred
    
    SPI1STATbits.SPIEN = 1; // Enable SPI module

    // Interrupt Controller Settings
    IFS0bits.SPI1IF = 0; // Clear the Interrupt Flag
    IEC0bits.SPI1IE = 1; // Enable the Interrupt
}

void avoidOverflow() {
    if (fillPos > TXBUFSIZE && sendPos > TXBUFSIZE) {
        fillPos -= TXBUFSIZE;
        sendPos -= TXBUFSIZE;
    }
    if(fillPos > sendPos + TXBUFSIZE){
        fillPos -= TXBUFSIZE;
    }
    if(sendPos > fillPos + TXBUFSIZE){
        sendPos -= TXBUFSIZE;
    }
}

void resetBuffer(){
    startSend = 0;
    sendPos = 0;
    fillPos = 0;
}

void __attribute__((interrupt, no_auto_psv)) _SPI1Interrupt(void) {
    //unsigned int buffer;
    //disableADCInt();

    // Retrieve data from receive buffer
    IFS0bits.SPI1IF = 0; //Clear the interrupt flag
    SPI1STATbits.SPIROV = 0; //Clear any errors
    //buffer = SPI1BUF;                //Read in SPI1 buffer

    // Transfers from dsPIC to ARM
    while(SPI1STATbits.SPITBF); // wait for transfer?

    int value = 0;
    if(startSend){
        if(sendPos < fillPos){
            value = txBuffer[sendPos++];
        }else{
            resetBuffer();
        }
    }
    WriteSPI1(value);

    //enableADCInt();
    return;
}

void spiStartSend(){
    startSend = 1;
}

void addToSPIBuffer(unsigned int data){
    txBuffer[fillPos] = data;

    fillPos++;
}
