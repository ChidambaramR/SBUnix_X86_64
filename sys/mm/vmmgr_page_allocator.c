#include <stdio.h>
#include <defs.h>
#include <sys/mm/mmgr.h>
#define VIRT_PAGE_SIZE 4096

static uint64_t vmmgr_memory_map[ (MY_KERNEL_SIZE*1024*1024)/(4096*64*10) ];

uint64_t vmmgr_max_blocks=0;
uint64_t vmmgr_used_blocks=0;
uint64_t vmmgr_usable_memory_size;

uint64_t vmmgr_get_total_blocks(){
        return vmmgr_max_blocks;
}

uint64_t vmmgr_get_used_blocks(){
        return vmmgr_used_blocks;
}

uint64_t vmmgr_get_total_usable_memory(){
        return vmmgr_usable_memory_size;
}

uint64_t vmmgr_get_total_usable_blocks(){
        return (vmmgr_get_total_blocks() - vmmgr_get_used_blocks());
}

/*
To say that the block represented by bit has been used. If bit 47 is passed,
then bit 47 in the 0th index of the array is set.
*/
inline void vmmgr_set_block (int bit) {
  vmmgr_memory_map[bit / 64] |= (1 << (bit % 64));
}


/*
Tests if the requested block is free or not.
*/
inline bool vmmgr_is_block_free (int bit) {
 return vmmgr_memory_map[bit / 64] &  (1 << (bit % 64));
}

/*
To say that the block represented by bit has is now free to use.
*/
inline void vmmgr_unset_block (int bit) {
  vmmgr_memory_map[bit / 64] &= ~ (1 << (bit % 64));
}

/*
Return 1st free block. Note that, each bit in the bit map array will represent a block.
So, say 132 blocks are filled and we are supposed to insert in the the 133rd block.
So it means that memory_map[0] and memory_map[1] will be all F's. memory_map[2] will be
0xFFFFFFFF000011111. Thus for j=5, memory_map[2] & 1<<5, the bit will be 0. Hence, we return
2*64 + 5 = 133rd block as free.
*/
int vmmgr_get_first_free(){
  uint32_t i,j;
  uint64_t bit;
  for(i=0; i< (vmmgr_get_total_blocks() / 64); i++){
    if(vmmgr_memory_map[i] != 0xFFFFFFFFFFFFFFFF){
      for(j=0; j<64; j++){
        bit = 1<<j;
        if( !(vmmgr_memory_map[i] & bit) )
          return i*64 + j;
      }
    }
  }
  return -1;
}

/*
Allocates a page from physical memory and returns the address at the physical memory.
Allocates only a single page
*/
void* vmmgr_alloc_block(){
  uint64_t page_frame;
  if(vmmgr_get_total_usable_blocks() <= 0){
    printf("Total usable blocks is less than or equal to 0\n");
    return NULL;
  }

  page_frame = vmmgr_get_first_free();
  if(page_frame == -1){
    printf("Not able to find a matching frame\n");
    return NULL;
  }

  vmmgr_set_block(page_frame);
  vmmgr_used_blocks++;

  //printf("First free page_frame = %d and its address is %p",page_frame,(page_frame * PHY_PAGE_SIZE));
  return ((void *)(page_frame * PHY_PAGE_SIZE));
}

void vmmgr_free_block(void *p){
  int page_frame = (((uint64_t)p)/BLOCK_SIZE);
  if(p == NULL){
    printf("You can't free a NULL pointer!\n");
    return;
  }
  vmmgr_unset_block(page_frame);
  vmmgr_used_blocks--;

}

void* vmmgr_page_alloc(){
    void* blk = vmmgr_alloc_block();
    return (void*)((uint64_t)blk + 0xFFFFFFFF81400000);
}

void vmmgr_print_stats(){
    printf("Total usable memory size = %d\n",vmmgr_get_total_usable_memory());
    printf("Max number of blocks = %d\n",vmmgr_get_total_blocks());
    printf("Total number of used blocks = %d\n",vmmgr_get_used_blocks());
    printf("Total number of free blocks = %d\n",vmmgr_get_total_usable_blocks());
}

void vmmgr_page_allocator_init(){
    uint64_t i;
    for(i = 0xFFFFFFFF81400000; i < 0xFFFFFFFF82C00000; i+=BLOCK_SIZE)
        vmmgr_max_blocks++;
    vmmgr_usable_memory_size = (vmmgr_max_blocks * VIRT_PAGE_SIZE)/(KILO_BYTE*KILO_BYTE);
    vmmgr_print_stats();
}
