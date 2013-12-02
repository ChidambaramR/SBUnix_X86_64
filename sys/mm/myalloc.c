#include <stdio.h>
#include <sys/mm/mmgr.h>
#include <sys/mm/vmmgr_virtual.h>
#include <defs.h>

/*
My own memory allocator. Suffers from Internal as well as external fragmentation.
Each size is aligned to nearest multiple of eight to make allocations simpler.
If the size asked is 9, then it is rounded to 16 bytes. Furhter to this control information
is added. Thus for a 9 byte block I am allocating 32 bytes. 16 bytes are for control info.
7 bytes are wasted due to internal fragmentation.
*/

// Structure for storing meta data
typedef struct metadata_block *t_block;

struct metadata_block{
t_block next; // Pointer to the next block
uint16_t size; // Size of the current chunk
uint16_t free;  // Indictaes if this block is free or not
//char beg;  // begining address of the block
};

t_block base = NULL;  //Base pointer
t_block last = NULL;  //Last pointer

#define align(x) (((x-1)>>3)<<3)+8    // To align the structure
//#define align(x) x

#define ALLOC_BLOCK_SIZE (sizeof(struct metadata_block))  // Size of our meta data block

// First fit algorithm
t_block find_block(t_block *last, uint16_t size){
        t_block b = base;
        while(b && !(b->free && (b->size >= size))){
                *last = b;
                b = b->next;
        }
        return b;  // Returns the address of a fitting chunk or NULL
}

//Extending the heap
t_block extend_heap(t_block last, uint16_t size){
        t_block b;
        b=vmmgr_alloc_page(0);
        if(!b)
            PANIC(__FUNCTION__,__LINE__,"No memory!");
        b->size = 0x1000;
        b->next = NULL;
        if(last)  // If the block is last , then point it to first and make the free pointer as 0
                last->next = b;
        b->free = 0;
        return (b);
}

void sub_free(void* p){
        t_block b;
        b = (t_block)((uint64_t)p - ALLOC_BLOCK_SIZE);
        b->free = 1;
}
//Split the block to avoid fragmentation
void split_block(t_block b,uint16_t size){
        uint16_t tot_size = size + ALLOC_BLOCK_SIZE;
        t_block new;
        new = (t_block)(b + 1 + tot_size/ALLOC_BLOCK_SIZE);
        new->next = b->next;
        new->size = b->size - tot_size;
        new->free = 1;
        b->size = size;
        b->next = new;

}

//Real malloc
void* sub_malloc(uint16_t size, bool align){
        t_block b;
        uint16_t s;
        if(align){
          void* addr = (void*)vmmgr_alloc_page(size);
          return addr;
        }
        else{
        s = align(size);  // Al gin it to a byte boundary
        if(base){
                last = base;
                b = find_block(&last,s); // Find the available block
                if(b){
                // A block can be split
                if((b->size - s) >= (ALLOC_BLOCK_SIZE + 4))
                        split_block(b,s);
                b->free = 0;
                }
                else{
                // No fitting block, so extend heap
                b = extend_heap(last,s);
                if(!b)
                        return NULL; // No space available
                }
        } else{
        // Coming in for first time
        b = extend_heap(last,s);
        if(!b)
                PANIC(__FUNCTION__,__LINE__,"No Memory!");
        base = b;
        split_block(b,s);
        }
        return (void*)((uint64_t)b + ALLOC_BLOCK_SIZE);
        }
}

