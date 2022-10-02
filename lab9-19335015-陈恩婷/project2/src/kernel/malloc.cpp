#include "malloc.h"

#include "os_type.h"
#include "stdio.h"
#include "sync.h"
#include "memory.h"
#include "os_constant.h"
#include "program.h"
#include "os_modules.h"

void MallocManager::initialize() {
    memory_blocks = 0;
    semaphore.initialize(1);
    printf("memory_blocks: %x\n", memory_blocks);
    printf("mallocManager: Initialized!\n");
}

void MallocManager::split(struct block *slot, size_t size) {
    //printf("%x %x\n", slot, size);
    struct block *new_block = (struct block *)((char *)(slot + 1) + size);
    //printf("%x\n", new_block);
    new_block->size = (slot->size) - size - sizeof(struct block);
    new_block->available = true;
    new_block->next = slot->next;
    slot->size = size;
    slot->available = false;
    slot->next = new_block;
    printf("mallocManager: One block split!\n");
}

void *MallocManager::malloc(size_t num_bytes) {
    if ( num_bytes > 4084 ){
        printf("Not enough memory to malloc!\n");
        return 0;
    }
    semaphore.P();
    if ( memory_blocks == 0 ){
        PCB *pcb = programManager.running;
        AddressPoolType poolType = (pcb->pageDirectoryAddress) ? AddressPoolType::USER : AddressPoolType::KERNEL;
        memory_blocks = (char *) memoryManager.allocatePages(poolType, 1);
        freeList = (struct block *)memory_blocks;
        freeList->size = PAGE_SIZE - sizeof(struct block);
        freeList->available = true;
        freeList->next = 0;
        // printf("freeList: %x\n", freeList);
        // printf("freeList->size: %d\n", freeList->size );
        // printf("freeList->available: %d\n", (int)freeList->available );
        // printf("freeList->next: %x\n", freeList->next );
    }
    struct block *curr;
    void *result;
    curr = freeList;
    while (curr->next != 0 && (curr->size < num_bytes || curr->available == 0)) {
        curr = curr->next;
    }
    if ( curr->available && curr->size == num_bytes) {
        curr->available = false;
        //printf("%x\n", curr);
        result = (void *)(curr + 1);
        //printf("%x %x %d\n", curr, result, sizeof(struct block));
        //printf("mallocManager: Block allocated!\n");
    }
    else if ( curr->available && curr->size > num_bytes + sizeof(struct block)) {
        split(curr, num_bytes);
        //printf("%x\n", curr);
        result = (void *)(curr + 1);
        //printf("%x %x %d\n", curr, result, sizeof(struct block));
        //printf("mallocManager: Block allocated!\n");
    }
    else {
        result = 0;
        //printf("malloManager: Haiyaa, not enough memory!\n");
    }
    semaphore.V();
    return result;
}

void MallocManager::merge() {
    struct block *curr;
    curr = freeList;
    while (curr && curr->next) {
        //printf( "%x %x %d %d\n", curr, curr->next, curr->available, curr->next->available );
        if (curr->available && curr->next->available) {
            curr->size += curr->next->size + sizeof(struct block);
            curr->next = curr->next->next;
            printf("mallocManager: Merged two blocks!\n");
        }
        curr = curr->next;
    }
}

void MallocManager::free(void *ptr) {
    semaphore.P();
    if ((void *)memory_blocks <= ptr && ptr <= (void *)(memory_blocks + PAGE_SIZE)) {
        struct block *curr = (struct block *)(ptr-1);
        curr->available = true;
        //printf("curr: %x\n", curr);
        merge();
        printf("mallocManager: One block freed!\n");
    }
    else{
        printf("mallocManager: Pointer invalid!\n");
    }
    semaphore.V();
}
