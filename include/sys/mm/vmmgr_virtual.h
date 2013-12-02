#ifndef _VMMGR_VIRTUAL_H
#define _VMMGR_VIRTUAL_H

#include <sys/mm/vmmgr_pte.h>
#include <sys/mm/vmmgr_pde.h>
#include <sys/mm/vmmgr_pdpe.h>
#include <sys/mm/vmmgr_pml4.h>

typedef uint64_t virtual_addr;

#define PAGES_PER_TABLE 512
#define PAGES_PER_DIR 512
#define VIRT_PAGE_SIZE 4096
// Refer AMD 64ABI
#define PAGE_TABLE_OFFSET(x) (x>>12 & 0x1FF)
#define PAGE_DIRECTORY_OFFSET(x) (x>>21 &0x1FF)
#define PAGE_POINTER_OFFSET(x) (x>>30 & 0x1FF)
#define PAGE_PML4_OFFSET(x) (x>>39 & 0x1FF)
#define PAGE_PHYSICAL_ADDRESS(x) (*x & ~0xFFF0000000000FFF) // Last three hex are always zero
                                               // due to alignment
/*
Level 4 ( lowest )
*/
struct page_table{
  pt_entry entry[PAGES_PER_TABLE];
};
typedef struct page_table pte;

/*
Level 3
*/
struct page_directory_table{
  pd_entry entry[PAGES_PER_DIR];
};
typedef struct page_directory_table pde;

/*
Level 2
*/
struct page_directory_pointer_table{
  pdpe_entry entry[PAGES_PER_DIR];
};
typedef struct page_directory_pointer_table pdpe;

/*
Level 1 ( Highest )
*/
struct page_map_level_4{
  pml4e_entry entry[PAGES_PER_DIR];
};
typedef struct page_map_level_4 pml4;

extern uint64_t get_cr3_register();

// Maps physical to virtual address
void vmmgr_map_page(virtual_addr, virtual_addr);

// Initialize virtual memory manager
void vmmgr_init();

// Allocates a page
void* vmmgr_alloc_page(uint16_t);

// Frees a page
void vmmgr_free_page(); 

void* sub_malloc(uint16_t, bool);

void sub_free(void*);
/*
Following functions gets the address of the directory table
*/

//Gets the address of the current PDE table
pde* vmmngr_get_page_directory();

//Gets the address of the current PDPE table
pdpe* vmmngr_get_page_pointer_directory();

// Gets the address of the current PML4 table
pml4* vmmngr_get_pml4_directory();


/*
Following functions switch to the given page directory
*/

// Switch to the specified page direcotry
bool vmmgr_switch_page_directory(pde *);

// Switch to the specified page directory pointer table
bool vmmgr_switch_page_pointer_directory(pdpe *);

// Switch to the specified PML4 direcotry
void vmmgr_switch_pml4_directory(pml4 *);


/*
Following functions converst a virtual address into a page table
index
*/

// Converts a virtual address to a page table index
uint32_t vmmgr_ptable_virt_to_index(virtual_addr);

// Convert virtual address to page directory index
uint32_t vmmgr_pdtable_virt_to_index(virtual_addr);

// Convert virtual address to page directory pointer index
uint32_t vmmgr_pdpointertable_virt_to_index(virtual_addr);

// Convert virtual address to pml4 index
uint32_t vmmgr_pml4_virt_to_index(virtual_addr);

/*
Following functions clear a directory structure
*/

// Clears a page table
void vmmgr_ptable_clear(pte *);

// Clears a page directory table
void vmmgr_page_directory_clear(pde *);

// Clears a page directory pointer table
void vmmgr_page_pointer_directory_clear(pdpe *);

// Clears a pml4 pable
void vmmgr_pml4_directory_clear(pml4 *);


/*
Following functions are used to lookup an entry in one of the table structures
*/

// Looks up a page table entry from a page table
pt_entry* vmmgr_ptable_lookup_entry(pte*, virtual_addr);

// Looks up a page direcotry entry from a page directory table
pd_entry* vmmgr_page_directory_lookup_entry(pde*, virtual_addr);

// Looks up a page pointer directory 
pdpe_entry* vmmgr_page_pointer_directory_lookup_entry(pdpe*, virtual_addr);

// Looks up a pml4 directory
pml4e_entry* vmmgr_pml4_directory_lookup_entry(pml4*, virtual_addr);

void* vmmgr_page_alloc();

void vmmgr_map_page_after_paging(uint64_t, uint64_t, bool);

void mmgr_syncwith_kernel();

void clone_pgdir(uint64_t, uint64_t);

 void copy_page_table(uint64_t, uint64_t, bool);

pd_entry* get_pde_offset_recurse(virtual_addr);

pt_entry* get_pte_recurse(virtual_addr);

void clear_page_tables(uint64_t);
#endif
