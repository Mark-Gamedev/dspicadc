#ifndef __PROCESSDATA__
#define __PROCESSDATA__

#define BUFFER_SZ 100
typedef fractional PPBType;

typedef struct {
    PPBType buf0[BUFFER_SZ];
    PPBType buf1[BUFFER_SZ];
    int fillCounter;
    int readCounter;
    int fill1read0;
}PingPongBuffer;

#endif
