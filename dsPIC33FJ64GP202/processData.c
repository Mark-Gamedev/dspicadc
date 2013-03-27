#include <dsp.h>
#include "processData.h"

PingPongBuffer ch0ppb;
PingPongBuffer ch1ppb;
PingPongBuffer ch2ppb;

ThresholdState thresholdState;

void ppbAdd(PingPongBuffer *ppb, PPBType value){
    if(ppb->fill1read0){
        ppb->buf1[ppb->fillCounter++] = value;
    }else{
        ppb->buf0[ppb->fillCounter++] = value;
    }
}

void ppbWriteFirst(PingPongBuffer *ppb, PPBType value){
    if(ppb->fill1read0){
        ppb->buf1[0] = value;
    }else{
        ppb->buf0[0] = value;
    }
}

int ppbIsFull(PingPongBuffer *ppb){
    return (ppb->fillCounter==BUFFER_SZ)?1:0;
}

void ppbSwitchBuffer(PingPongBuffer *ppb){
    ppb->fill1read0 ^= 1;
    ppb->fillCounter = 0;
}

int isCh0Full(){
    return ppbIsFull(&ch0ppb);
}

int isCh1Full(){
    return ppbIsFull(&ch1ppb);
}

int isCh2Full(){
    return ppbIsFull(&ch2ppb);
}

void switchBufferCh0(){
    ppbSwitchBuffer(&ch0ppb);
}

void switchBufferCh1(){
    ppbSwitchBuffer(&ch1ppb);
}

void switchBufferCh2(){
    ppbSwitchBuffer(&ch2ppb);
}

void addToChannel0(int value) {
    if (thresholdState == StartStoring) {
        ppbAdd(&ch0ppb, (PPBType) value);
    } else {
        ppbWriteFirst(&ch0ppb, (PPBType) value);
    }
}

void addToChannel1(int value) {
    if (thresholdState == StartStoring) {
        ppbAdd(&ch1ppb, (PPBType) value);
    } else {
        ppbWriteFirst(&ch1ppb, (PPBType) value);
    }
}

void addToChannel2(int value) {
    if (thresholdState == StartStoring) {
        ppbAdd(&ch2ppb, (PPBType) value);
    } else {
        ppbWriteFirst(&ch2ppb, (PPBType) value);
    }
}

void performXCorrelation(){
    PPBType *ch0ptr = ch0ppb.fill1read0 ? ch0ppb.buf0 : ch0ppb.buf1;
    PPBType *ch1ptr = ch1ppb.fill1read0 ? ch1ppb.buf0 : ch1ppb.buf1;
    PPBType *ch2ptr = ch2ppb.fill1read0 ? ch2ppb.buf0 : ch2ppb.buf1;

    fractional result[BUFFER_SZ*2-1];
    int corr01, corr02, corr12;
    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, result, ch0ptr, ch1ptr);
    VectorMax(BUFFER_SZ*2-1, result, &corr01);
    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, result, ch0ptr, ch2ptr);
    VectorMax(BUFFER_SZ*2-1, result, &corr02);
    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, result, ch1ptr, ch2ptr);
    VectorMax(BUFFER_SZ*2-1, result, &corr12);
    addToSPIBuffer(0xAB00 | corr01);
    //addToSPIBuffer(0xAB00 | corr02);
    //addToSPIBuffer(0xAB00 | corr12);
    spiStartSend();
}
