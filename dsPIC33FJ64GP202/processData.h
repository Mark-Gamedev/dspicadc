#ifndef __PROCESSDATA__
#define __PROCESSDATA__

#include <dsp.h>

typedef fractional PPBType;
//buffer size has to be power of 2 in order to avoid modula
#define BUFFER_MASK 0x01FF
#define BUFFER_SZ BUFFER_MASK+1

typedef struct {
    PPBType buf0[BUFFER_SZ];
    PPBType buf1[BUFFER_SZ];
    int fillCounter;
    int readCounter;
    int fill1read0;
}PingPongBuffer;

//Circular Buffer
typedef fractional CBType;

#define CBModBufferSz(x) (x & BUFFER_MASK)

typedef struct {
    CBType buf[BUFFER_SZ];
    int start;
    int count;
}CircularBuffer;

CBType cbRead(CircularBuffer*);
void cbWrite(CircularBuffer*, CBType);
int cbIsEmpty(CircularBuffer*);
int cbIsFull(CircularBuffer*);

typedef int ThresholdState;
#define BelowThreshold 0
#define AboveThreshold 1
#define StartStoring   2
extern ThresholdState thresholdState;

#endif
