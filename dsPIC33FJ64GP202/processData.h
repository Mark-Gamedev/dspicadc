#ifndef __PROCESSDATA__
#define __PROCESSDATA__

#include <dsp.h>

/*  Settings  */
//buffer size has to be power of 2 in order to avoid modula
#define BUFFER_MASK 0x01FF         // 2^9 = 512
#define BUFFER_SZ BUFFER_MASK+1
/* Q15 format
 * 0.500 -> 16384
 * 0.525 -> 17203
 * 0.550 -> 18022
 * 0.575 -> 18842
 * 0.600 -> 19661
 * 0.625 -> 20480
 * 0.650 -> 21299
 * 0.675 -> 22118
 * 0.700 -> 22938
 * 0.725 -> 23757
 * 0.750 -> 24576
 * 0.775 -> 25395
 * 0.800 -> 26214
 */
#define THRESHOLD 19661

//Circular Buffer
typedef fractional CBType;

#define CBModBufferSz(x) (x & BUFFER_MASK)

typedef struct {
    CBType buf[BUFFER_SZ];
    int start;
    int size;
    int count;
}CircularBuffer;

CBType cbRead(CircularBuffer*);
void cbWrite(CircularBuffer*, CBType);
int cbIsEmpty(CircularBuffer*);
int cbIsFull(CircularBuffer*);

#endif
