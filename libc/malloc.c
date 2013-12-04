#include<stdio.h>
#include <ustdlib.h>
// Structure for storing meta data
typedef struct metadata_block *t_block;

struct metadata_block{
int size; // Size of the current chunk
t_block next; // Pointer to the next block
int free;  // Indictaes if this block is free or not
uint64_t beg;  // begining address of the block
};

void *base;  //Base pointer
int start = 0;

#define align(x) (((x-1)>>3)<<3)+8    // To align the structure 

#define BLOCK_SIZE sizeof(struct metadata_block)  // Size of our meta data block

// First fit algorithm
t_block find_block(t_block *last, int size){
	t_block b = base;
	while(b && !(b->free && (b->size >= size))){
		*last = b;
		b = b->next;
	} 
	return b;  // Returns the address of a fitting chunk or NULL
}

//Extending the heap
t_block extend_heap(t_block last, int size){
	t_block b;
//        printf("okay here\n");
	b=(t_block)((uint64_t)sbrk(0));
  //      printf("add b = %d\n",b);
	sbrk(BLOCK_SIZE + size); 
	b->size = size;
	b->next = NULL;
	if(last)  // If the block is last , then point it to first and make the free pointer as 0
		last->next = b;
	b->free = 0;
	return (b);
}

//Split the block to avoid fragmentation
void split_block(t_block b,int size){
	t_block new;
	new = (t_block)(b->beg + size);
	new->next = b->next;
	new->size = b->size - size - BLOCK_SIZE;
	new->free = 1;
	b->size = size;
	b->next = new;
	
}

//Real malloc
void* malloc(int size){
	t_block b,last;
	int s;
	s = align(size);  // Algin it to a byte boundary
        last = NULL;
        if(start){
          base = NULL;
          start++;
        }
        printf("size = %d\n",s);
	if(base){
		last = base;
		b = find_block(&last,s); // Find the available block
		if(b){
		// A block can be split
		if((b->size - s) >= (BLOCK_SIZE + 4))
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
        //printf("okay herenn\n");
	b = extend_heap(last,s);
	if(!b)
		return NULL;
	base = b;
	}
	return (void*)(b);
}

