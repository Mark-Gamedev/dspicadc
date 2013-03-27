#include <dsp.h>
#include "processData.h"

CircularBuffer ch0cb;
CircularBuffer ch1cb;
CBType ch0ptr[BUFFER_SZ];
CBType ch1ptr[BUFFER_SZ];

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

CBType cbRead(CircularBuffer *cb){
    CBType ret = cb->buf[cb->start];
    cb->start = CBModBufferSz(cb->start + 1);
    cb->count--;
}

int cbIsFull(CircularBuffer *cb){
    return cb->count == BUFFER_SZ;
}

int cbIsEmpty(CircularBuffer *cb){
    return cb->count == 0;
}

void cbWrite(CircularBuffer *cb, CBType val){
    int end = CBModBufferSz(cb->start + cb->count);
    cb->buf[end] = val;
    if (cb->count == BUFFER_SZ){
        CBModBufferSz(cb->start + 1);
    }else{
        cb->count++;
    }
}

int isCh0Full(){
}

int isCh1Full(){
}

int isCh2Full(){
}

void switchBufferCh0(){
}

void switchBufferCh1(){
}

void switchBufferCh2(){
}

void addToChannel0(int value) {
    cbWrite(&ch0cb, value);
}

void addToChannel1(int value) {
    cbWrite(&ch1cb, value);
}

void addToChannel2(int value) {
}

void performXCorrelation(){
    fractional result[BUFFER_SZ*2-1];

    int i=0;
    for(;i<BUFFER_SZ;i++){
        ch0ptr[i] = cbRead(&ch0cb);
        ch1ptr[i] = cbRead(&ch1cb);
    }

    int corr01;
    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, result, ch0ptr, ch1ptr);
    VectorMax(BUFFER_SZ*2-1, result, &corr01);
    return;
}
