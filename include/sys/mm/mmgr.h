#ifndef _MMGR_H
#define _MMGR_H

#define BLOCKS_PER_BYTE 8
#define KILO_BYTE 1024
#define MEGA_BYTE (1024*1024)
#define BLOCK_SIZE (4*KILO_BYTE)
#define BLOCK_ALIGN BLOCK_SIZE
#define PHY_PAGE_SIZE BLOCK_SIZE

struct smap_t {
      uint64_t base, length;
      uint32_t type;
}__attribute__((packed));

void mm_phy_init(uint32_t *);

void mmgr_free_size_blocks(void*, int);

void mmgr_free_block(void *);

void* mmgr_alloc_size_blocks(int);

void* mmgr_alloc_block();

void mmgr_print_memory_status();
/*enum memory_errors{
    E_ALLOC_SUCCES = 0,
    E_ALLOC_OUT_OF_MEMORY = -1
};

typedef enum memory_errors memory_return_status;
*/
#endif
