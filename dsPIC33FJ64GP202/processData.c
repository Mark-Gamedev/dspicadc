#include <dsp.h>
#include <string.h>
#include "processData.h"

CircularBuffer cbch0;
CircularBuffer cbch1;
CircularBuffer cbch2;
CBType xcorrBuf0[BUFFER_SZ];
CBType xcorrBuf1[BUFFER_SZ];
fractional xcorrResult[BUFFER_SZ*2-1];

CBType cbRead(CircularBuffer *cb){
    CBType ret = cb->buf[cb->start];
    cb->start = CBModBufferSz(cb->start + 1);
    cb->count--;
    int part1, part2;
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
        CBModBufferSz(cb->start + 1);
    }else{
        cb->count++;
    }
}

void cbCopyToArray(CircularBuffer *cb, CBType *dst){
    int size;
    size = (cb->size - cb->start)*sizeof(CBType);
    memcpy(dst, &(cb->buf[cb->start]), size);
    size = cb->start*sizeof(CBType);
    memcpy(dst+size, cb->buf, size);
}

void initBuffers(){
    cbch0.size = BUFFER_SZ;
    cbch1.size = BUFFER_SZ;
    cbch2.size = BUFFER_SZ;
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

void performXCorrelation(){
    int corr01;

    cbCopyToArray(&cbch0, xcorrBuf0);
    cbCopyToArray(&cbch1, xcorrBuf1);

    int spiData[2];

    VectorCorrelate(BUFFER_SZ, BUFFER_SZ, xcorrResult, xcorrBuf0, xcorrBuf1);
    VectorMax(BUFFER_SZ*2-1, xcorrResult, &corr01);
    spiData[0] = 0xFF01;
    spiData[1] = corr01;
    spiSendWordArrayBlocking(spiData, 2);
}
