typedef uint64_t virtual_addr;

#define PAGES_PER_TABLE 512
#define PAGES_PED_DIR 512

/*
Level 4 ( lowest )
*/
struct page_table{
  pt_entry m_entires[PAGES_PER_TABLE];
};
typedef struct page_table pte;

/*
Level 3
*/
struct page_directory_table{
  pd_entry m_entries[PAGES_PED_DIR];
};
typedef page_directory_table pde;

/*
Level 2
*/
struct page_directory_pointer_table{
  pdpe_entry m_entries[PAGES_PED_DIR];
};
typedef page_directory_pointer_table pdpe;

/*
Level 1 ( Highest )
*/
struct page_map_level_4{
  pml4e_entry m_entries[PAGES_PER_DIR];
};
typedef page_map_level_4 pml4;

// Maps physical to virtual address
void vmmgr_map_page(void *, viod *);

// Initialize virtual memory manager
void vmmgr_init();

// Allocates a page
bool vmmgr_alloc_page();

// Frees a page
void vmmgr_free_page(); 


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
bool vmmgr_switch_pml4_directory(pml4 *);


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
pd_entry* vmmgr_page_directory_lookup_entry(pte*, virtual_addr);

// Looks up a page pointer directory 
pdpe_entry* vmmgr_page_pointer_directory_lookup_entry(pdpe*, virtual_addr);

// Looks up a pml4 directory
pml4e_entry* vmmgr_pml4_directory_lookup_entry(pml4*, virtual_addr);
