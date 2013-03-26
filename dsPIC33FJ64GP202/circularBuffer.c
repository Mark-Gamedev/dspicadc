#include <stdio.h>

/* Opaque buffer element type.  This would be defined by the application. */
typedef struct {
    int value;
} ElemType;

/* Circular buffer object */
typedef struct {
    int size;        /* maximum number of elements           */
    int start;       /* index of oldest element              */
    int count;       /* number of elements                   */
    ElemType *elems; /* vector of elements                   */
} CircularBuffer;

void cbInit(CircularBuffer *cb, int size) {
    cb->size = size;
    cb->start = 0;
    cb->count = 0;
    cb->elems = (ElemType *) calloc(cb->size, sizeof (ElemType));
}

int cbIsFull(CircularBuffer *cb) {
    return cb->count == cb->size;
}

int cbIsEmpty(CircularBuffer *cb) {
    return cb->count == 0;
}

void cbWrite(CircularBuffer *cb, ElemType *elem) {
    int end = (cb->start + cb->count) % cb->size;
    cb->elems[end] = *elem;
    if (cb->count == cb->size){
        cb->start = (cb->start + 1) % cb->size; /* full, overwrite */
    }else{
        cb->count++;
    }
}

void cbRead(CircularBuffer *cb, ElemType *elem) {
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
    cb->count--;
}
