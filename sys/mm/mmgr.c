#include <defs.h>
#include <stdio.h>
#include <sys/mm/mmgr.h>
#include <sys/mm/regions.h>
#include <stdlib.h>

/*
Caution getting system specific
*/

/*
structure which maintains total memory to teh system given by the e820 map
*/
struct smap_t sys_total_memory[10];
uint64_t total_mem_counter = 0;

/*
Structure which maintains available memory to the system.
*/
struct smap_t sys_usable_memory[10];
uint64_t usable_mem_counter = 0;

/*
_mmnger_memory_map is a pointer to the bit map structure that we use to keep track of all of physical memory.
Each bit is a 0 if that block has not been allocated (Useable) or a 1 if it is reserved (In use).
 The number of bits in this array is _mmngr_max_blocks. In other words, each bit represents a single memory block,
 which in turn, is 4KB of physical memory.

_mmngr_max_blocks containes the amount of memory blocks available. This is the size of physical memory
 (Retrieved from the BIOS from the boot loader) divide by PMMNGR_BLOCK_SIZE. This essentally divides the
 physical address space into memory blocks

_mmngr_used_blocks containes the amount of blocks currently in use

 _mmngr_memory_size is for refrence only--it containes the amount of physical memory in KB
*/

//! size of physical memory available to the kernel
static  uint64_t  mmgr_usable_memory_size=0;

//! number of blocks currently in use
static  uint64_t  mmgr_used_blocks=0;

//! maximum number of available memory blocks
static  uint64_t  mmgr_max_blocks=0;

/* memory map bit array. Each bit represents a memory block
Total number of blocks = 127MB / Block_size. i.e 127MB / 4K ~= 32600
No of blocks which can be represented in a byte = 8, because one bit
  represent each block. Thus we need a 32600 / 8 ~= 4100 bit map.
  i.e we need 4100 bytes roughly to represent all the blocks.
On using a 64 bit integer array, each entry in the array can represent
  64 blocks. Thus the size of the bit map array = 4100 / 8 ~= 600.
  Thus I am allocating 600 bytes for the bit map array.
Multiplying by 2 to support 256 MB of RAM
*/

/*
Caution, getting system specific
*/
static  uint64_t mmgr_memory_map[1200];

uint64_t get_total_blocks(){
        return mmgr_max_blocks;
}

uint64_t get_used_blocks(){
        return mmgr_used_blocks;
}

uint64_t get_total_usable_memory(){
        return mmgr_usable_memory_size;
}

uint64_t get_total_usable_blocks(){
        return (get_total_blocks() - get_used_blocks());
}
void print_total_system_memory(){
        uint16_t i;
        for(i=0; i<total_mem_counter; i++){
             printf("Base %x\tLength %x\ttype %d\n",sys_total_memory[i].base,sys_total_memory[i].length,sys_total_memory[i].type); 
        } 
}


void print_usable_system_memory(){
        uint16_t i;
        for(i=0; i<usable_mem_counter; i++){
             printf("Base %x\tLength %x\ttype %d\n",sys_usable_memory[i].base,sys_usable_memory[i].length,sys_usable_memory[i].type); 
        } 
        printf("\n\n Totable usable memory of the system = %d\tTotal number of blocks = %d\n",get_total_usable_memory(),get_total_blocks());
}


/*
To say that the block represented by bit has been used. If bit 47 is passed,
then bit 47 in the 0th index of the array is set.
*/
inline void mmgr_set_block (int bit) {
  mmgr_memory_map[bit / 64] |= (1 << (bit % 64));
}


/*
Tests if the requested block is free or not.
*/
inline bool mmgr_is_block_free (int bit) {
 return mmgr_memory_map[bit / 64] &  (1 << (bit % 64));
}

/*
To say that the block represented by bit has is now free to use.
*/
inline void mmgr_unset_block (int bit) {
  mmgr_memory_map[bit / 64] &= ~ (1 << (bit % 64));
}

void mmgr_print_memory_status(){
  printf("\nTotal usable memory = %d\n",get_total_usable_memory());
  printf("Total number of blocks = %d\n",get_total_blocks());
  printf("Total number of used blocks = %d\n",get_used_blocks());
  printf("Total number of usable blocks = %d\n\n",get_total_usable_blocks()); 
}

/*
Return 1st free block. Note that, each bit in the bit map array will represent a block.
So, say 132 blocks are filled and we are supposed to insert in the the 133rd block.
So it means that memory_map[0] and memory_map[1] will be all F's. memory_map[2] will be
0xFFFFFFFF000011111. Thus for j=5, memory_map[2] & 1<<5, the bit will be 0. Hence, we return
2*64 + 5 = 133rd block as free.
*/
int mmgr_get_first_free(){
  uint32_t i,j;
  uint64_t bit;
  for(i=0; i< (get_total_blocks() / 64); i++){
    if(mmgr_memory_map[i] != 0xFFFFFFFFFFFFFFFF){
      for(j=0; j<64; j++){
        bit = 1<<j;
        if( !(mmgr_memory_map[i] & bit) )
          return i*64 + j;
      }
    }
  }
  return -1;
}


int mmgr_get_first_range_free(int size){
  uint32_t i,j,count;
  uint64_t bit;
  for(i=0; i< (get_total_blocks() / 64); i++){
      if(mmgr_memory_map[i] != 0xFFFFFFFFFFFFFFFF){
          for(j=0; j<64; j++){
              bit = 1<<j;
              if(!(mmgr_memory_map[i] & bit)){
                  uint32_t temp_bit = i*64; // Go to that corresponding frame
                  uint32_t free = 0;
                  temp_bit += j;
                  for(count=0; count<size; count++){
                      if( !(mmgr_is_block_free(temp_bit + count)) )
                          free++;
                      else{
                          j=j+(count);
                          break; // No use of being in this loop anymore
                      }
                      if(free == size)
                          return ((i*64)+j);  // Found the required range. Return it
                  }
              }
          }
      }
  }
  return -1;
}

/*
Allocates a page from physical memory and returns the address at the physical memory.
Allocates only a single page
*/
void* mmgr_alloc_block(){
  uint64_t page_frame;
  if(get_total_usable_blocks() <= 0){
    printf("Total usable blocks is less than or equal to 0\n");
    return NULL;
  }

  page_frame = mmgr_get_first_free();
  //printf("rpf = %d ",page_frame);
  if(page_frame == -1){
    printf("Not able to find a matching frame\n");
    return NULL;
  }

  mmgr_set_block(page_frame);
  mmgr_used_blocks++;

  //printf("First free page_frame = %d and its address is %p",page_frame,(page_frame * PHY_PAGE_SIZE));
  return ((void *)(page_frame * PHY_PAGE_SIZE));
}

void* mmgr_alloc_size_blocks(int size){
  uint64_t page_frame;
  int i;
  if(get_total_usable_blocks() <= size){
    printf("Total usable blocks is less than or equal to 0\n");
    return NULL;
  }
  
  page_frame = mmgr_get_first_range_free(size);
  if(page_frame == -1){
    printf("Not able to find a matching frame\n");
    return NULL;
  }

  for(i=0; i<size; i++){
    mmgr_set_block(page_frame + i);     
  }

  mmgr_used_blocks += size;

  return ((void *)(page_frame * PHY_PAGE_SIZE));
}
  

void mmgr_free_block(void *p){
  int page_frame = (((uint64_t)p)/BLOCK_SIZE);
  if(p == NULL){
    printf("You can't free a NULL pointer!\n");
    return;
  }
  mmgr_unset_block(page_frame);
  mmgr_used_blocks--;
  
}

void mmgr_free_size_blocks(void *p, int size){
  int page_frame = (((uint64_t)p)/BLOCK_SIZE);
  int i;
  if(p == NULL){
    printf("You can't free a NULL pointer!\n");
    return;
  }
  for(i=0; i<size; i++)
    mmgr_unset_block(page_frame + i);
  mmgr_used_blocks -= size;
  
}
/*
Initialize the memory regiion so that it can be used. Set it to 0, i.e free a block
Subtract the used_blocks counter. Initially all blocks are assumed to be used and 
as and when we free a block, we decrement the used blocks. So when we are done
initializing all the blocks, then the usable block counter will be 0. 
*/
void mmgr_phy_init_regions(uint64_t base, uint64_t length){
        uint64_t block_no = base / BLOCK_SIZE;
        uint64_t blocks = length / BLOCK_SIZE;
        for(; blocks>0; blocks--){
          mmgr_unset_block(block_no++);
          mmgr_used_blocks--;
        }
        /*
        If we dont set the 1st block as set, then alloc will return 0 as the address for
        the first frame. It will confuse with NULL pointer. So we allocate only from the 
        1st block.
        */
//        mmgr_set_block(0); // Kernel pages
        /*
        We allocated an 8192 byte array to keep track of free pages. It needs approx 3
        physical pages. So 
        */
        
}

/*
Deinitialize the memory region so that it cannot be used. Basically we are
setting the corresponding bit map to 1, to indicate that the region of memory
is being used. Increment the number of blocks to keep track of how many blocks 
have actually been deemed unusable.
*/
void mmgr_phy_deinit_regions(uint64_t base, uint64_t length){
        uint64_t block_no = base / BLOCK_SIZE;
        uint64_t blocks = length / BLOCK_SIZE;
        for(; blocks>0; blocks--){
          mmgr_set_block(block_no++);
          mmgr_used_blocks++;
        }
        mmgr_set_block(0);
}
void mmgr_syncwith_kernel(){
        uint16_t i;
        for(i=512; i< ((MY_KERNEL_SIZE*256)); i++){
          mmgr_set_block(i); 
          mmgr_used_blocks += 1;
        }
}

void mm_phy_init(uint32_t* modulep){
        struct smap_t *smap;
        uint16_t i;
        while(modulep[0] != 0x9001) modulep += modulep[1]+2;
        for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
                if(smap->length != 0){
                        sys_total_memory[total_mem_counter].base = smap->base;
                        sys_total_memory[total_mem_counter].length = smap->length;
                        sys_total_memory[total_mem_counter].type = smap->type;
                        if(smap->type == MM_PHY_USABLE){
                          mmgr_usable_memory_size += smap->length;
                          sys_usable_memory[usable_mem_counter].base = smap->base;
                          sys_usable_memory[usable_mem_counter].length = smap->length;
                          sys_usable_memory[usable_mem_counter].type = smap->type;
                          usable_mem_counter++; 
                        }
                        total_mem_counter++;
                }
        }
        mmgr_max_blocks = (mmgr_usable_memory_size/(BLOCK_SIZE));
        mmgr_used_blocks = mmgr_max_blocks;
        mmgr_usable_memory_size = mmgr_usable_memory_size/(MEGA_BYTE);
        //print_total_system_memory();
        //printf("\n\n");
        memset(mmgr_memory_map,0xF,((mmgr_max_blocks)/64));
    //    printf("\n\n1. Usable blocks = %d",get_total_usable_blocks());
        for(i=0; i<usable_mem_counter; i++){
            mmgr_phy_init_regions(sys_usable_memory[i].base,sys_usable_memory[i].length);
        } 

        /*
        When the loop runs 256 times, we cover blocks of size 1MB. Check in vmmgr_virtual.c
        where we allocated virtual memory space for kernel which is about 40MB size.
        Thus 40*256 = 10240. When the loop runs 10240 times, we cover 40MB size. We make
        all those blocks allocated as they will be used exclusively by the kernel. Allocations
        will start after 40MB mark. 
        */
        for(i=0; i< 1024; i++){
          mmgr_set_block(i); 
          mmgr_used_blocks += 1;
        }
      
}


inline uint16_t mmgr_get_block_count(){
  return 0;
}

