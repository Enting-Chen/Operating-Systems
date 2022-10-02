//malloc.h

#ifndef MALLOC_H
#define MALLOC_H

#include "os_type.h"
#include "stdio.h"
#include "sync.h"

struct block {
    size_t size;
    bool available;
    struct block *next;
};

class MallocManager{
private:
    char * memory_blocks;
    struct block *freeList = (struct block *)memory_blocks;
    Semaphore semaphore;
public:
    void initialize();
    void split(struct block*, size_t);
    void *malloc(size_t);
    void merge();
    void free(void *);           
};

#endif
