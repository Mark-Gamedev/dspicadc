#ifndef __PROCESSDATA__
#define __PROCESSDATA__

#include <dsp.h>
#define BUFFER_SZ 400
typedef fractional PPBType;

typedef struct {
    PPBType buf0[BUFFER_SZ];
    PPBType buf1[BUFFER_SZ];
    int fillCounter;
    int readCounter;
    int fill1read0;
}PingPongBuffer;

typedef int ThresholdState;
#define BelowThreshold 0
#define AboveThreshold 1
#define StartStoring   2
extern ThresholdState thresholdState;
#endif
