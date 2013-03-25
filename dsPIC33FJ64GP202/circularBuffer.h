#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER
#define  BUFFERSZ	1000
/* Opaque buffer element type.  This would be defined by the application. 
typedef struct {
    int value;
} ElemType;
*/

/* Circular buffer object */
typedef struct {
    int size;        /* maximum number of elements           */
    int start;       /* index of oldest element              */
    int count;       /* number of elements                   */
    int *elems; /* vector of elements                   */
} CircularBuffer;
extern CircularBuffer *cb;
#endif
