#include <dsp.h>
#include <string.h>
#include "processData.h"

CircularBuffer cbch0;
CircularBuffer cbch1;
CircularBuffer cbch2;
fractional dataBuffer[BUFFER_SZ*3];
fractional *ch0Buf;
fractional *ch1Buf;
fractional *ch2Buf;
/*
fractional *xcorrBuf0;
fractional *xcorrBuf1;
fractional *xcorrResult;
*/

CBType cbRead(CircularBuffer *cb){
    CBType ret = cb->buf[cb->start];
    cb->start = CBModBufferSz(cb->start + 1);
    cb->count--;
    return ret;
}

int cbIsFull(CircularBuffer *cb){
    return cb->count == cb->size;
}

int cbIsEmpty(CircularBuffer *cb){
    return cb->count == 0;
}

void cbWrite(CircularBuffer *cb, CBType val){
    int end = CBModBufferSz(cb->start + cb->count);
    cb->buf[end] = val;
    if (cb->count == cb->size){
        cb->start = CBModBufferSz(cb->start + 1);
    }else{
        cb->count++;
    }
}

void cbCopyToArray(CircularBuffer *cb, CBType *dst){
    int size0, size1;
    void* dst1;
    size0 = (cb->size - cb->start)*sizeof(CBType);
    size1 = cb->start*sizeof(CBType);
    dst1 = dst;
    dst1+=size0;
    memcpy(dst, &(cb->buf[cb->start]), size0);
    memcpy(dst1, cb->buf, size1);
}

void initBuffers() {
    cbch0.size = BUFFER_SZ;
    cbch1.size = BUFFER_SZ;
    cbch2.size = BUFFER_SZ;

    ch0Buf = &dataBuffer[0];
    ch1Buf = &dataBuffer[BUFFER_SZ];
    ch2Buf = &dataBuffer[BUFFER_SZ*2];

    /*
    xcorrBuf0   = &dataBuffer[0];
    xcorrBuf1   = &dataBuffer[BUFFER_SZ];
    xcorrResult = &dataBuffer[BUFFER_SZ*2];
    */
}

int isCh0Full(){
    cbIsFull(&cbch0);
}

int isCh1Full(){
    cbIsFull(&cbch1);
}

int isCh2Full(){
    cbIsFull(&cbch2);
}

void addToChannel0(int value) {
    cbWrite(&cbch0, value);
}

void addToChannel1(int value) {
    cbWrite(&cbch1, value);
}

void addToChannel2(int value) {
    cbWrite(&cbch2, value);
}

void readChannel0(){
    cbRead(&cbch0);
}

void readChannel1(){
    cbRead(&cbch1);
}

void readChannel2(){
    cbRead(&cbch2);
}

void sendBufferOverSpi(){
    cbCopyToArray(&cbch0, ch0Buf);
    cbCopyToArray(&cbch1, ch1Buf);
    cbCopyToArray(&cbch2, ch2Buf);
    
    /* send data over spi */
    spiSendWordArrayBlocking(ch0Buf, BUFFER_SZ*3);
}

/*
void performXCorrelation(){
    int corr01;

    cbCopyToArray(&cbch0, xcorrBuf0);
    cbCopyToArray(&cbch1, xcorrBuf1);

    int spiData[3];
    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, xcorrResult, xcorrBuf0, xcorrBuf1);
    VectorMax(BUFFER_SZ*2-1, xcorrResult, &corr01);
    spiData[0] = 0xAB01;
    spiData[1] = corr01;
    spiData[2] = 0;
    spiSendWordArrayBlocking(spiData, 3);
    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, xcorrResult, xcorrBuf1, xcorrBuf0);
    VectorMax(BUFFER_SZ*2-1, xcorrResult, &corr01);
    spiData[0] = 0xAB10;
    spiData[1] = corr01;
    spiData[2] = 0;
    spiSendWordArrayBlocking(spiData, 3);
}
*/