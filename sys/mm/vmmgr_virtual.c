#include <sys/mm/vmmgr_virtual.h>
#include <sys/mm/mmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <defs.h>
extern void vmmgr_load_pml4(uint64_t);
extern char kernmem, physbase;
extern void* kphysfree;
uint64_t bump_addr = 0;
//Current Page directory table
pde* _cur_pde_directory=0;

// Current page directory pointer table
pdpe* _cur_pdpe_directory=0;

// Current pml4 directory table
pml4* _cur_pml4_directory=0;

// Pointer to physical address of pml4 base address
pml4* _cur_pml4_base_pointer=0;

// Returns the index of the block in the page table
pt_entry* vmmgr_ptable_lookup_entry(pte* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_TABLE_OFFSET(addr) ]);
    }
    return NULL;
}

pd_entry* vmmgr_page_directory_lookup_entry(pde* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_DIRECTORY_OFFSET(addr) ]);
    }
    return NULL;
}


pdpe_entry* vmmgr_page_pointer_directory_lookup_entry(pdpe* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_POINTER_OFFSET(addr) ]);
    }
    return NULL;
}


pml4e_entry* vmmgr_pml4_directory_lookup_entry(pml4* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_PML4_OFFSET(addr) ]);
    }
    return NULL;
}


inline void vmmgr_switch_pml4_directory(pml4* p){
    if(!p){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Page directory to be switched not found\n");
        return;
    }
    _cur_pml4_directory = p;
    vmmgr_load_pml4( ((uint64_t)p ));
}

pml4* vmmgr_get_current_pml4_directory(){
    return _cur_pml4_directory;
}

void init_bump_addr(){
    bump_addr = (0xFFFFFFFF80000000 + (uint64_t)kphysfree);
}
inline void* vmmgr_alloc_page(){
    bump_addr = (bump_addr + 0x1000);
    return (void *)bump_addr;
}

inline void vmmgr_free_page(){
}

void set_pml4_entry(pml4e_entry* e, pdpe* pdpe_dir){
    memset(pdpe_dir, 0, sizeof(pdpe));
    pml4e_entry_add_attrib(e, PML4E_PRESENT);
    pml4e_entry_add_attrib(e, PML4E_WRITABLE);
    pml4e_entry_set_frame(e, (uint64_t)(pdpe_dir));
}

void set_pdpe_entry(pdpe_entry* e1, pde* pde_dir){
    memset(pde_dir, 0, sizeof(pde));
    pdpe_entry_add_attrib(e1, PDPE_PRESENT);
    pdpe_entry_add_attrib(e1, PDPE_WRITABLE);
    pdpe_entry_set_frame(e1, (uint64_t)(pde_dir));
}

void set_pde_entry(pd_entry* e2, pte* pte_dir){
    memset(pte_dir, 0, sizeof(pte_dir));
    pd_entry_add_attrib(e2, PDE_PRESENT);
    pd_entry_add_attrib(e2, PDE_WRITABLE);
    pd_entry_set_frame(e2, (uint64_t)(pte_dir));
}

/*
IMPORTANT! Understand this before reading the code. There are 4 tables
in total. Each page has 512 entries. The top level page PML4 contains 512
entries. Each entry has the base address of a third level table. Thus each 
entry points to the base address of a third level page table. 
The third level page table which this entry points in turn has 512 entries where
each entry has the base address of a 2nd level page table and this applies for the
innter most level.
*/
void vmmgr_map_page(uint64_t phys, uint64_t virt){
    pdpe* pdpe_dir1;
    pde* pde_dir1;
    pte* pte_dir1;
    pt_entry* page;
    pd_entry* pde_entry1;
    pdpe_entry* pdpe_entry1; 
    pml4* pml4_dir = (pml4*)_cur_pml4_base_pointer; // vmmgr_get_current_pml4_directory();
    pml4e_entry* e = vmmgr_pml4_directory_lookup_entry(pml4_dir, (uint64_t)virt);

    if( !(*e & PML4E_PRESENT) ){
        pdpe* pdpe_dir = (pdpe*)mmgr_alloc_block(); // create upper middle level directory
        if(! pdpe_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDPE\n");
            return;
        }
        set_pml4_entry(e,pdpe_dir);
    }
    
    pdpe_dir1 = (pdpe*)PAGE_PHYSICAL_ADDRESS(e);
    pdpe_entry1 = vmmgr_page_pointer_directory_lookup_entry(pdpe_dir1, (uint64_t)virt);

    if( !(*pdpe_entry1 & PDPE_PRESENT) ){
        pde* pde_dir = (pde*)mmgr_alloc_block(); // Create lower middle level directory
        if( !pde_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDE\n");
            return;
        }
        set_pdpe_entry(pdpe_entry1,pde_dir);
    }

    pde_dir1 = (pde*)PAGE_PHYSICAL_ADDRESS(pdpe_entry1);
    pde_entry1 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);

    if( !(*pde_entry1 & PDE_PRESENT) ){
        pte* pte_dir = (pte*)mmgr_alloc_block(); // Create lower level page table
        if( !pte_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PTE\n");
            return;
        }
        pd_entry*   e2 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);
        set_pde_entry(e2,pte_dir);
    }

    pte_dir1 = (pte*)PAGE_PHYSICAL_ADDRESS(pde_entry1);
    page = vmmgr_ptable_lookup_entry(pte_dir1, (uint64_t)virt);
    pt_entry_set_frame(page, (uint64_t)phys);
    pt_entry_add_attrib(page, PTE_PRESENT);
    pt_entry_add_attrib(page, PTE_WRITABLE);
    
}

void vmmgr_init(){
    int i;
    uint64_t frame, virt;

    pml4* pml4table = (pml4*)mmgr_alloc_block();
    if(!pml4table){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory which trying to allocate a default page\n");
        return;
    }
    memset(pml4table,0,sizeof(pml4));
    _cur_pml4_base_pointer = pml4table;

    // Map kernel's segment
    for(i=0, frame=0x200000, virt=(0xFFFFFFFF80000000+(uint64_t)(&physbase)); i<PAGES_PER_TABLE; i++, frame=frame+PHY_PAGE_SIZE, virt+=VIRT_PAGE_SIZE){
       vmmgr_map_page(frame, virt); 
    }
    for(i=0; i<((MY_KERNEL_SIZE/2)*PAGES_PER_TABLE); i++, frame=frame+PHY_PAGE_SIZE, virt+=VIRT_PAGE_SIZE){
       vmmgr_map_page(frame, virt); 
    }
    vmmgr_map_page(0xB8000, 0xFFFFFFFF80100000); 
    vmmgr_switch_pml4_directory(pml4table);
    init_bump_addr();
}
