#include <sys/mm/vmmgr_virtual.h>
#include <defs.h>

//Current Page directory table
pde* _cur_pde_directory=0;

// Current page directory pointer table
pdpe* _cur_pdpe_directory=0;

// Current pml4 directory table
pml4* _cur_pml4_directory=0;

// Pointer to physical address of pml4 base address
uint32_t _cur_pml4_base_pointer=0;

// Returns the index of the block in the page table
inline pt_entry* vmmgr_ptable_lookup_entry(pte* p, virtual_addr addr){
    if(p){
        return p->entry[ PAGE_TABLE_OFFSET(addr) ];
    }
    return NULL;
}

inline pd_entry* vmmgr_page_directory_lookup_entry(pde* p, virtual_addr){
    if(p){
        return p->entry[ PAGE_TABLE_OFFSET(addr) ];
    return NULL;
}


inline pdpe_entry* vmmgr_page_pointer_directory_lookup_entry(pdpe* p, virtual_addr){
    if(p){
        return p->entry[ PAGE_TABLE_OFFSET(addr) ];
    }
    return NULL;
}


inline pdpe_entry* vmmgr_pml4_directory_lookup_entry(pml4* p, virtual_addr){
    if(p){
        return p->entry[ PAGE_TABLE_OFFSET(addr) ];
    }
    return NULL;
}


inline bool vmmgr_switch_pml4_directory(pml4* p){
    if(!p){
        return FALSE;

    _cur_pml4_directory = p;

    vmmgr_load_pml4(_cur_pml4_base_pointer);
}
