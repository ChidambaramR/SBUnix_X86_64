#include <sys/mm/vmmgr_virtual.h>
#include <sys/mm/mmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <defs.h>
extern void vmmgr_load_pml4(uint64_t);
extern char kernmem, physbase;

//Current Page directory table
pde* _cur_pde_directory=0;

// Current page directory pointer table
pdpe* _cur_pdpe_directory=0;

// Current pml4 directory table
pml4* _cur_pml4_directory=0;

// Pointer to physical address of pml4 base address
uint64_t _cur_pml4_base_pointer=0;

// Returns the index of the block in the page table
inline pt_entry* vmmgr_ptable_lookup_entry(pte* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_TABLE_OFFSET(addr) ]);
    }
    return NULL;
}

inline pd_entry* vmmgr_page_directory_lookup_entry(pde* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_DIRECTORY_OFFSET(addr) ]);
    }
    return NULL;
}


inline pdpe_entry* vmmgr_page_pointer_directory_lookup_entry(pdpe* p, virtual_addr addr){
    if(p){
        return &(p->entry[ PAGE_POINTER_OFFSET(addr) ]);
    }
    return NULL;
}


inline pml4e_entry* vmmgr_pml4_directory_lookup_entry(pml4* p, virtual_addr addr){
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

inline pml4* vmmgr_get_current_pml4_directory(){
    return _cur_pml4_directory;
}

inline bool vmmngr_alloc_page(){
    return TRUE;
}

inline void vmmgr_free_page(){
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
inline void vmmgr_map_page(void* phys, void *virt){
    pdpe* pdpe_dir1;
    pde* pde_dir1;
    pte* pte_dir1;
    pt_entry* page;
    pd_entry* pde_entry1;
    pdpe_entry* pdpe_entry1; 
    pml4* pml4_dir = vmmgr_get_current_pml4_directory();
    //pml4e_entry* e = &pml4->entry[PAGE_PML4_OFFSET(virt)];
    pml4e_entry* e = vmmgr_pml4_directory_lookup_entry(pml4_dir, (uint64_t)virt);
    //pdpe_entry* e1 = &pdpe->entry[PAGE_POINTER_OFFSET(virt)];
    //pd_entry* e2 = &pde->entry[PAGE_DIRECTORY_OFFSET(virt)];

    if( !(*e & PML4E_PRESENT) ){
        // Top level page is missing. Allocate 3 levels of memory
        /*
        Create the third level page table and set the parameters
        in the 4th level. Book-keeping stuff.
        */
        /*
        If you see, we are allocating a 4KB block for each table. the
        reason is that, each table has 512 entries. Each entry is 64bit.
        Thus for a table as a whole, we need 512*8 = 4096 bytes which is
        4KB. Through each entry of the table we can manage a 4KB physical
        page. The physical address of a page is stored in the 64 bit entry.
        */
        pdpe* pdpe_dir = (pdpe*)mmgr_alloc_block();
        if(! pdpe_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDPE\n");
            return;
        }
        memset(pdpe_dir, 0, sizeof(pdpe));
        pml4e_entry_add_attrib(e, PML4E_PRESENT);
        pml4e_entry_add_attrib(e, PML4E_WRITABLE);
        pml4e_entry_set_frame(e, (uint64_t)(pdpe_dir));
        
        /*
        Create second level page table and set the parameters in the
        3rd level. Book-keeping stuff.
        */
        pde* pde_dir = (pde*)mmgr_alloc_block();
        if( !pde_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDE\n");
            return;
        }
        memset(pde_dir, 0, sizeof(pde));
        pdpe_entry* e1 = vmmgr_page_pointer_directory_lookup_entry(pdpe_dir, (uint64_t)virt);
        pdpe_entry_add_attrib(e1, PDPE_PRESENT);
        pdpe_entry_add_attrib(e1, PDPE_WRITABLE);
        pdpe_entry_set_frame(e1, (uint64_t)(pde_dir));

        /*
        Create the lowest level page table and set the parameters
        in the 2nd level. Book-keeping stuff.
        */
        pte* pte_dir = (pte*)mmgr_alloc_block();
        if( !pte_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PTE\n");
            return;
        }
        memset(pte_dir, 0, sizeof(pte_dir));
        pd_entry* e2 = vmmgr_page_directory_lookup_entry(pde_dir, (uint64_t)virt);
        pd_entry_add_attrib(e2, PDE_PRESENT);
        pd_entry_add_attrib(e2, PDE_WRITABLE);
        pd_entry_set_frame(e2, (uint64_t)(pte_dir));
    }
    // Phew! Top level page entry is present
    // Get physical address of page pointer directory table  
    pdpe_dir1 = (pdpe*)PAGE_PHYSICAL_ADDRESS(e);
    pdpe_entry1 = vmmgr_page_pointer_directory_lookup_entry(pdpe_dir1, (uint64_t)virt);

    pde_dir1 = (pde*)PAGE_PHYSICAL_ADDRESS(pdpe_entry1);
    pde_entry1 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);

    pte_dir1 = (pte*)PAGE_PHYSICAL_ADDRESS(pde_entry1);
    page = vmmgr_ptable_lookup_entry(pte_dir1, (uint64_t)virt);

    pt_entry_set_frame(page, (uint64_t)phys);
    pt_entry_add_attrib(page, PTE_PRESENT);
    
}

void vmmgr_init(){
    int i;
    uint64_t frame, virt;
    /* 
    Allocate a default page table. It gets a physical page of size 4096 bytes,
    We then typecast it to the structure of type pte which has 512 entries, each
    of size 8 bytes. Thus the total size is 512*8 = 4096 bytes. The allocated
    block will essentially contain 512 entries, each of size 8 bytes. The same
    applies for all the other tables too.  
    */
    pte* ptable = (pte*)mmgr_alloc_block(); 
    pte* ptable1 = (pte*)mmgr_alloc_block(); 
    if( !ptable || !ptable1){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory which trying to allocate a default page\n");
        return;
    }
    memset(ptable,0,sizeof(pte));
    memset(ptable1,0,sizeof(pte));

    /*
    We are creating two page directory tables because we are mapping two virtual
    address range here. 0x00.. and 0xFF. So they will be managed by two different 
    page directory tables.
    */
    pde* pdtable = (pde*)mmgr_alloc_block();
    if( !pdtable ){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory which trying to allocate a default page\n");
        return;
    }
    memset(pdtable,0,sizeof(pde));

    /*
    We are creating two page directory tables because we are mapping two virtual
    address range here. 0x00.. and 0xFF. So they will be managed by two different 
    page directory tables.
    */
    pdpe* pdptable = (pdpe*)mmgr_alloc_block();
    if( !pdptable ){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory which trying to allocate a default page\n");
        return;
    }
    memset(pdptable,0,sizeof(pdpe));

    pml4* pml4table = (pml4*)mmgr_alloc_block();
    if(!pml4table){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory which trying to allocate a default page\n");
        return;
    }
    memset(pml4table,0,sizeof(pml4));

    /* Identity mapping 1st 2MB in physical memory to virtual memory
    How? We have 512 entries per page table. Each entry manages 4KB of physical memory.
    Thus 512 entries manage 512*4096 = 2MB of physical memory. This 2MB memory is
    identity mapped into our kernel page table.
    */
    /*
    Why? Identity mapping. We map two virtual ranges to the same physical address range. Thus when 
    some memory mapped IO is performed, the address will be 0xB8000 and it will be considered as 
    virtual address. We should map this one too. Else we will get page fault.
    */
    pt_entry page=0;
    pt_entry_add_attrib(&page, PTE_PRESENT);
    pt_entry_set_frame(&page, (uint64_t)0xB8000);
    ptable->entry[ PAGE_TABLE_OFFSET(0xFFFFFFFF80400000) ] = page;

    /*
    Important place. We are mapping our own pages which was mapped by boot loader to
    the page table we are going to create. Any miss in this place will probable
    cause a triple fault.
    */ 
    for(i=0, frame=0x200000, virt=(0xFFFFFFFF80000000+physbase); i<PAGES_PER_TABLE; i++, frame=frame+PHY_PAGE_SIZE, virt+=VIRT_PAGE_SIZE){
        pt_entry page=0;
        pt_entry_add_attrib(&page, PTE_PRESENT);
        pt_entry_add_attrib(&page, PTE_WRITABLE);
        pt_entry_set_frame(&page, (uint64_t)(frame));
        ptable1->entry[ PAGE_TABLE_OFFSET(virt) ] = page;
    }

    // Done cretaing the page table. Now map it across the levels.

    /*
    Map the two page tables to the corresponding page directory tables. We are creating
    two tables because, both the virtual address does not fall in the same page pointer
    directory offset in the virtual address. i.e The page pointer directory offset for
    both these virtual address entries are different. 
    */

    pd_entry* pdentry1 = &pdtable->entry[ PAGE_DIRECTORY_OFFSET(0xFFFFFFFF80200000) ];
    pd_entry_add_attrib( pdentry1, PDE_PRESENT);
    pd_entry_add_attrib( pdentry1, PDE_WRITABLE);
    pd_entry_set_frame( pdentry1, (uint64_t)((uint64_t)ptable1));

    pd_entry* pdentry = &pdtable->entry[ PAGE_DIRECTORY_OFFSET(0xFFFFFFFF80400000) ];
    pd_entry_add_attrib( pdentry, PDE_PRESENT);
    pd_entry_add_attrib( pdentry, PDE_WRITABLE);
    pd_entry_set_frame( pdentry, (uint64_t)((uint64_t)ptable));
    
    /*
    Map the two page tables to the corresponding page directory pointer tables.Just one table
    is enough because, the page pointer offset for both the addresses are the same. Thus the
    page pointer offset in the previous level ( which is same for both virt address ), points
    to the same page directory table. Also we need to create more than 512 page tables to create
    another page directory. In this case we are creating only two page tables and hence just
    one page directory is enough. 
    */
    pdpe_entry* pdpentry = &(pdptable->entry[ PAGE_POINTER_OFFSET(0xFFFFFFFF80200000) ]);
    pdpe_entry_add_attrib( pdpentry, PDE_PRESENT);
    pdpe_entry_add_attrib( pdpentry, PDE_WRITABLE);
    pdpe_entry_set_frame( pdpentry, (uint64_t)((uint64_t)pdtable));

    /*pdpe_entry* pdpentry1 = &pdptable1->entry[ PAGE_POINTER_OFFSET(0xFFFFFFFF80200000) ];
    pdpe_entry_add_attrib( pdentry, PDE_PRESENT);
    pdpe_entry_add_attrib( pdentry, PDE_WRITABLE);
    pdpe_entry_set_frame( pdentry, (uint32_t)pdtable1);*/

    /*
    We add these to 1 pml4 table only. There can be only one PML4 table.
    */
    pml4e_entry* pml4entry1 = &(pml4table->entry[ PAGE_PML4_OFFSET(0xFFFFFFFF80200000) ]);
    pml4e_entry_add_attrib( pml4entry1, PML4E_PRESENT );
    pml4e_entry_add_attrib( pml4entry1, PML4E_WRITABLE );
    pml4e_entry_set_frame( pml4entry1, (uint64_t)((uint64_t)pdptable));
  
    _cur_pml4_base_pointer = (uint64_t)(&(pml4table->entry));
    vmmgr_switch_pml4_directory(pml4table);

    /*
    Important routine.
    */
    //paging_enable();
}
