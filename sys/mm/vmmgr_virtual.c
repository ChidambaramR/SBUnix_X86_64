#include <sys/mm/vmmgr_virtual.h>
#include <sys/mm/mmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <defs.h>

#define RECURSIVE_ENTRY 0xFFFFFF7FBFDFE000
#define RECURSIVE_PAGE_TABLE_ENTRY 0xFFFFFF0000000000
#define RECURSIVE_PAGE_DIRECTORY_ENTRY 0xFFFFFF7F80000000
#define RECURSIVE_PAGE_POINTER_ENTRY 0xFFFFFF7fBFC00000
 
extern void vmmgr_load_pml4(uint64_t);
extern char kernmem, physbase;
extern void* kphysfree;
uint64_t bump_addr = 0;
uint64_t bump_start = 0;
uint64_t kernel_pgd = 0;
uint64_t virt_base = 0xFFFFFFFF80000000;
uint16_t prev_no_pages = 0;
//Current Page directory table
pde* _cur_pde_directory=0;

// Current page directory pointer table
pdpe* _cur_pdpe_directory=0;

// Current pml4 directory table
pml4* _cur_pml4_directory=0;

// Pointer to physical address of pml4 base address
pml4* _cur_pml4_base_pointer=0;

pml4* _cur_pml4_base_pointer_pg=(pml4*)RECURSIVE_ENTRY;

/*uint64_t get_phy_from_virt(uint64_t virt){
    return(virt - (uint64_t)0xFFFFFFFF80000000);
    
}
*/

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
    //bump_start = bump_addr;
    bump_addr = (uint64_t)((uint64_t)0xFFFFFFFF80000000 + (uint64_t)kphysfree);
}
inline void* vmmgr_alloc_page(uint16_t page){
    bump_addr = (bump_addr + VIRT_PAGE_SIZE + VIRT_PAGE_SIZE*prev_no_pages);
    prev_no_pages = page;
    return (void *)bump_addr;
}

inline void vmmgr_free_page(){
}

void set_pml4_entry_recurs(pml4e_entry* e, pml4* pml4_dir, bool is_user){
    pml4e_entry_add_attrib(e, PML4E_PRESENT);
    pml4e_entry_add_attrib(e, PML4E_WRITABLE);
    if(is_user)
      pml4e_entry_add_attrib(e, PML4E_USER);
    pml4e_entry_set_frame(e, (uint64_t)(pml4_dir));
}


void set_page_entry(pt_entry* e, uint64_t phys, bool is_user){
    pml4e_entry_add_attrib(e, PTE_PRESENT);
    pml4e_entry_add_attrib(e, PTE_WRITABLE);
    if(is_user)
      pml4e_entry_add_attrib(e, PTE_USER);
    pml4e_entry_set_frame(e, phys);
}

void set_pml4_entry(pml4e_entry* e, pdpe* pdpe_dir, bool is_user){
    pml4e_entry_add_attrib(e, PML4E_PRESENT);
    pml4e_entry_add_attrib(e, PML4E_WRITABLE);
    if(is_user)
      pml4e_entry_add_attrib(e, PML4E_USER);
    pml4e_entry_set_frame(e, (uint64_t)(pdpe_dir));
}

void set_pdpe_entry(pdpe_entry* e1, pde* pde_dir, bool is_user){
    pdpe_entry_add_attrib(e1, PDPE_PRESENT);
    pdpe_entry_add_attrib(e1, PDPE_WRITABLE);
    if(is_user)
      pdpe_entry_add_attrib(e1, PDPE_USER);
    pdpe_entry_set_frame(e1, (uint64_t)(pde_dir));
}

void set_pde_entry(pd_entry* e2, pte* pte_dir, bool is_user){
    pd_entry_add_attrib(e2, PDE_PRESENT);
    pd_entry_add_attrib(e2, PDE_WRITABLE);
    if(is_user)
      pd_entry_add_attrib(e2, PDE_USER);
    pd_entry_set_frame(e2, (uint64_t)(pte_dir));
}

pml4e_entry* get_pml4e_offset_recurse(virtual_addr virt){
    pml4* pml4_dir = _cur_pml4_base_pointer_pg;
    pml4e_entry* e = (pml4e_entry*)((uint64_t)pml4_dir + (uint64_t)(8*(PAGE_PML4_OFFSET(virt))));
    return e;
}

pdpe_entry* get_pdpe_offset_recurse(virtual_addr virt){
    pdpe* pdpe_dir1 = (pdpe*)RECURSIVE_PAGE_POINTER_ENTRY;
    uint64_t pdpe_offset = (((uint64_t)(pdpe_dir1) >> 12) + (PAGE_PML4_OFFSET(virt))); 
    pdpe_entry* pdpe_entry1 = (pdpe_entry*)(((uint64_t)(pdpe_offset) << 12) + (uint64_t)((8*(PAGE_POINTER_OFFSET(virt)))));
    return pdpe_entry1;
}

pd_entry* get_pde_offset_recurse(virtual_addr virt){
    pde* pde_dir1 = (pde*)RECURSIVE_PAGE_DIRECTORY_ENTRY;
    uint64_t pde_offset = (((uint64_t)(pde_dir1) >> 21) + (PAGE_PML4_OFFSET(virt)));
    pde_offset = (pde_offset<<21);
    pde_offset = (((uint64_t)(pde_offset) >> 12) + (PAGE_POINTER_OFFSET(virt)));
    pd_entry* pde_entry1 = (pd_entry*)(((uint64_t)(pde_offset) << 12) + (uint64_t)((8*(PAGE_DIRECTORY_OFFSET(virt)))));
    return pde_entry1;
}

void set_phys_virt_recurse(uint64_t phys, virtual_addr virt, bool is_user){
    pte *pte_dir1;
    uint64_t pte_offset;
    pt_entry* pte_entry1;
    pte_dir1 = (pte*)RECURSIVE_PAGE_TABLE_ENTRY;
    pte_offset = (((uint64_t)(pte_dir1) >> 30) + (PAGE_PML4_OFFSET(virt)));
    pte_offset = (pte_offset<<30);
    pte_offset = (((uint64_t)(pte_offset) >> 21) + (PAGE_POINTER_OFFSET(virt)));
    pte_offset = (pte_offset<<21);
    pte_offset = (((uint64_t)(pte_offset) >> 12) + (PAGE_DIRECTORY_OFFSET(virt)));
    pte_entry1 = (pt_entry*)(((uint64_t)(pte_offset) << 12) + (uint64_t)((8*(PAGE_TABLE_OFFSET(virt)))));
    //*pte_entry1 = (uint64_t)phys;
    set_page_entry(pte_entry1, phys, is_user);
    return;
}

pt_entry* get_pte_recurse(virtual_addr virt){
    pte *pte_dir1;
    uint64_t pte_offset;
    pt_entry* pte_entry1;
    pte_dir1 = (pte*)RECURSIVE_PAGE_TABLE_ENTRY;
    pte_offset = (((uint64_t)(pte_dir1) >> 30) + (PAGE_PML4_OFFSET(virt)));
    pte_offset = (pte_offset<<30);
    pte_offset = (((uint64_t)(pte_offset) >> 21) + (PAGE_POINTER_OFFSET(virt)));
    pte_offset = (pte_offset<<21);
    pte_offset = (((uint64_t)(pte_offset) >> 12) + (PAGE_DIRECTORY_OFFSET(virt)));
    pte_entry1 = (pt_entry*)(((uint64_t)(pte_offset) << 12) + (uint64_t)((8*(PAGE_TABLE_OFFSET(virt)))));
    return pte_entry1;

}
void clone_pgdir(uint64_t parent_pml4, uint64_t child_pml4){
    uint16_t i;
    pml4e_entry* ppml4 = (pml4e_entry*)parent_pml4;
    pml4e_entry* cpml4 = (pml4e_entry*)child_pml4;
    for(i=0; i<512; i++){
      cpml4[i] = ppml4[i];    
    }
    cpml4[510] = (uint64_t)cpml4 - 0xFFFFFFFF80000000;
    cpml4[510] = cpml4[510] | PTE_USER | PTE_PRESENT | PTE_WRITABLE;
    //printf("cpml4 510 = %x cpml4 = %p\n",cpml4[510],cpml4);
}

void copy_page_table(uint64_t parent_pml4, uint64_t child_pml4, bool is_user){
    uint16_t i,j,k,l;
    pde* pde_dir;
    pte* pte_dir;
    pdpe* pdpe_dir;
    pd_entry *pd, *pde_dir_beg;
    pt_entry* pg, *pte_dir_beg;
    pml4e_entry* ppml4 = (pml4e_entry*)parent_pml4;
    pdpe_entry *ppdpe, *pdpe_dir_beg;
    pml4e_entry* cpml4 = (pml4e_entry*)child_pml4;
    for(i=0; ppml4[i] != NULL; i++){
        pdpe_dir = (pdpe*)sub_malloc(0,1); // create upper middle level directory
        pdpe_dir_beg = (pdpe_entry*)pdpe_dir;
        memset(pdpe_dir, 0, sizeof(pdpe));
        pdpe* pdpe_dir_phy = (pdpe*)((uint64_t)pdpe_dir - (uint64_t)0xFFFFFFFF80000000);
        if(! pdpe_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDPE\n");
            return;
        }
        set_pml4_entry(&cpml4[i],pdpe_dir_phy, is_user);
        
        // Second level
        ppdpe = (pdpe_entry*)PAGE_PHYSICAL_ADDRESS(&ppml4[i]);
        ppdpe = (pdpe_entry*)((uint64_t)ppdpe + virt_base);
        for(j=0; ppdpe[j] != NULL; j++){
            pde_dir = (pde*)sub_malloc(0,1); // Create lower middle level directory
            pde_dir_beg = (pd_entry*)pde_dir;
            memset(pde_dir, 0, sizeof(pde));
            pde* pde_dir_phy = (pde*)((uint64_t)pde_dir - (uint64_t)0xFFFFFFFF80000000);
            if( !pde_dir ){
                PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDE\n");
                return;
            }
            set_pdpe_entry((pdpe_entry*)&pdpe_dir_beg[j],pde_dir_phy, is_user);

            // Third level
            pd = (pd_entry*)PAGE_PHYSICAL_ADDRESS(&ppdpe[j]);
            pd = (pd_entry*)((uint64_t)pd + virt_base);
            for(k=0; k < 512; k++){
                if(pd[k]){
                    pte_dir = (pte*)sub_malloc(0,1); // Create lower level page table
                    pte_dir_beg = (pt_entry*)pte_dir;
                    memset(pte_dir, 0, sizeof(pte_dir));
                    pte* pte_dir_phy = (pte*)((uint64_t)pte_dir - (uint64_t)0xFFFFFFFF80000000);
                    if( !pte_dir ){
                        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PTE\n");
                        return;
                    }
                    set_pde_entry((pd_entry*)&pde_dir_beg[k],pte_dir_phy, is_user);
                    memcpy((char*)pte_dir, (const char*)(virt_base + (uint64_t)PAGE_PHYSICAL_ADDRESS(&pd[k])), sizeof(pte));
                    pg = (pt_entry*)PAGE_PHYSICAL_ADDRESS(&pd[k]);
                    pg = (pd_entry*)((uint64_t)pg + virt_base);
                    for(l=0; l<512; l++){
                        pt_entry_del_attrib(&pte_dir_beg[l], PTE_WRITABLE); 
                        pt_entry_del_attrib(&pg[l], PTE_WRITABLE); 
                        pt_entry_add_attrib(&pg[l], PTE_COW); 
                        pt_entry_add_attrib(&pte_dir_beg[l], PTE_COW); 
                    }
                    // To add COW bits
                }
            }    
        }
    }
}



void clear_page_tables(uint64_t parent_pml4){
    uint16_t i,j,k;
    pd_entry *pd, *pd_phy;
    pt_entry *pg, *pg_phy;
    pml4e_entry* ppml4 = (pml4e_entry*)parent_pml4;
    pdpe_entry *ppdpe, *ppdpe_phy;
    for(i=0; ppml4[i] != NULL; i++){
        
        // Second level
        ppdpe_phy = (pdpe_entry*)PAGE_PHYSICAL_ADDRESS(&ppml4[i]);
        ppdpe = (pdpe_entry*)((uint64_t)ppdpe_phy + virt_base);
        ppml4[i] = 0;
        for(j=0; ppdpe[j] != NULL; j++){
            // Third level
            pd_phy = (pd_entry*)PAGE_PHYSICAL_ADDRESS(&ppdpe[j]);
            pd = (pd_entry*)((uint64_t)pd_phy + virt_base);
            ppdpe[j] = 0;
            for(k=0; k < 512; k++){
                if(pd[k]){
                    pg_phy = (pt_entry*)PAGE_PHYSICAL_ADDRESS(&pd[k]);
                    pg = (pt_entry*)((uint64_t)pg_phy + virt_base);
                    memset(pg, 0, VIRT_PAGE_SIZE);
                    pd[k] = 0;
                    }
                    // To add COW bits
            }
        }
    }
}


void vmmgr_map_page_after_paging(uint64_t phys, uint64_t virt, bool is_user){
    pdpe *pdpe_dir;
    pd_entry* pde_entry1;
    pdpe_entry* pdpe_entry1;

    pml4e_entry* e = get_pml4e_offset_recurse(virt);//(pml4e_entry*)((uint64_t)pml4_dir + (uint64_t)(8*(PAGE_PML4_OFFSET(virt))));

    if( !(*e & PML4E_PRESENT) ){
        pdpe_dir = (pdpe*)sub_malloc(0,1); // create upper middle level directory
        memset(pdpe_dir, 0, sizeof(pdpe));
        pdpe* pdpe_dir_phy = (pdpe*)((uint64_t)pdpe_dir - (uint64_t)0xFFFFFFFF80000000);
        if(! pdpe_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDPE\n");
            return;
        }
        set_pml4_entry(e,pdpe_dir_phy, is_user);
    }
    
    pdpe_entry1 = get_pdpe_offset_recurse(virt);

    if( !(*pdpe_entry1 & PDPE_PRESENT) ){
        pde* pde_dir = (pde*)sub_malloc(0,1); // Create lower middle level directory
        memset(pde_dir, 0, sizeof(pde));
        pde* pde_dir_phy = (pde*)((uint64_t)pde_dir - (uint64_t)0xFFFFFFFF80000000);
        if( !pde_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDE\n");
            return;
        }
        set_pdpe_entry(pdpe_entry1,pde_dir_phy, is_user);
    }

    pde_entry1 = get_pde_offset_recurse(virt);

    if( !(*pde_entry1 & PDE_PRESENT) ){
        pte* pte_dir = (pte*)sub_malloc(0,1); // Create lower level page table
        memset(pte_dir, 0, sizeof(pte_dir));
        pte* pte_dir_phy = (pte*)((uint64_t)pte_dir - (uint64_t)0xFFFFFFFF80000000);
        if( !pte_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PTE\n");
            return;
        }
        set_pde_entry(pde_entry1,pte_dir_phy, is_user);
    }
    
    set_phys_virt_recurse(phys, virt, is_user);
    
}

/*
Return the physical address of the Page table corresponding to address lAddress
*/
/*pte* get_page_table_address(pml4* pml4_dir, uint64_t virt)
{
    pdpe* pdpe_dir1;
    pde* pde_dir1;
    pte* pte_dir1;
    pd_entry* pde_entry1;
    pdpe_entry* pdpe_entry1; 

    pml4e_entry* e = vmmgr_pml4_directory_lookup_entry(pml4_dir, (uint64_t)virt);
    if( !(*e & PML4E_PRESENT) ){
        pdpe* pdpe_dir = (pdpe*)sub_malloc(0,1); // create upper middle level directory
        if(! pdpe_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDPE\n");
            return NULL;
        }
        set_pml4_entry(e,pdpe_dir);
    }
    
    pdpe_dir1 = (pdpe*)PAGE_PHYSICAL_ADDRESS(e);
    pdpe_entry1 = vmmgr_page_pointer_directory_lookup_entry(pdpe_dir1, (uint64_t)virt);

    if( !(*pdpe_entry1 & PDPE_PRESENT) ){
        pde* pde_dir = (pde*)sub_malloc(0,1); // Create lower middle level directory
        if( !pde_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDE\n");
            return NULL;
        }
        set_pdpe_entry(pdpe_entry1,pde_dir);
    }

    pde_dir1 = (pde*)PAGE_PHYSICAL_ADDRESS(pdpe_entry1);
    pde_entry1 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);

    if( !(*pde_entry1 & PDE_PRESENT) ){
        pte* pte_dir = (pte*)sub_malloc(0,1); // Create lower level page table
        if( !pte_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PTE\n");
            return NULL;
        }
        pd_entry*   e2 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);
        set_pde_entry(e2,pte_dir);
    }

    pte_dir1 = (pte*)PAGE_PHYSICAL_ADDRESS(pde_entry1);
    return pte_dir1;
}
*/

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
        memset(pdpe_dir, 0, sizeof(pdpe));
        if(! pdpe_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDPE\n");
            return;
        }
        set_pml4_entry(e,pdpe_dir,0);
    }
    
    pdpe_dir1 = (pdpe*)PAGE_PHYSICAL_ADDRESS(e);
    pdpe_entry1 = vmmgr_page_pointer_directory_lookup_entry(pdpe_dir1, (uint64_t)virt);

    if( !(*pdpe_entry1 & PDPE_PRESENT) ){
        pde* pde_dir = (pde*)mmgr_alloc_block(); // Create lower middle level directory
        memset(pde_dir, 0, sizeof(pde));
        if( !pde_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PDE\n");
            return;
        }
        set_pdpe_entry(pdpe_entry1,pde_dir,0);
    }

    pde_dir1 = (pde*)PAGE_PHYSICAL_ADDRESS(pdpe_entry1);
    pde_entry1 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);

    if( !(*pde_entry1 & PDE_PRESENT) ){
        pte* pte_dir = (pte*)mmgr_alloc_block(); // Create lower level page table
        memset(pte_dir, 0, sizeof(pte_dir));
        if( !pte_dir ){
            PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory while trying to allocate PTE\n");
            return;
        }
        pd_entry*   e2 = vmmgr_page_directory_lookup_entry(pde_dir1, (uint64_t)virt);
        set_pde_entry(e2,pte_dir,0);
    }

    pte_dir1 = (pte*)PAGE_PHYSICAL_ADDRESS(pde_entry1);
    page = vmmgr_ptable_lookup_entry(pte_dir1, (uint64_t)virt);
    pt_entry_set_frame(page, (uint64_t)phys);
    pt_entry_add_attrib(page, PTE_PRESENT);
    pt_entry_add_attrib(page, PTE_WRITABLE);
    
}

/*pml4* create_pgd(int pid, int orig_pid){
    return NULL;
}*/

void vmmgr_init(){
    int i;
    uint64_t frame, virt;

    pml4* pml4table = (pml4*)mmgr_alloc_block();
    if(!pml4table){
        PANIC(__FUNCTION__,__LINE__,"VMMGR: Out of memory which trying to allocate a default page\n");
        return;
    }
    memset(pml4table,0,sizeof(pml4));
    pml4e_entry* e= &(pml4table->entry[0x1FE]);
    set_pml4_entry_recurs(e, pml4table, 0); 
    
    _cur_pml4_base_pointer = pml4table;

    // Map kernel's segment
    for(i=0, frame=0x200000, virt=(0xFFFFFFFF80000000+(uint64_t)(&physbase)); i<PAGES_PER_TABLE; i++, frame=frame+PHY_PAGE_SIZE, virt+=VIRT_PAGE_SIZE){
       vmmgr_map_page(frame, virt); 
    }
    for(i=0; i<((MY_KERNEL_SIZE/2)*PAGES_PER_TABLE); i++, frame=frame+PHY_PAGE_SIZE, virt+=VIRT_PAGE_SIZE){
       vmmgr_map_page(frame, virt); 
    }
    vmmgr_map_page(0xB8000, 0xFFFFFFFF80100000); 
    vmmgr_map_page((uint64_t)pml4table, 0xFFFFFF7FBFDFE000);
    kernel_pgd = 0xFFFFFF7FBFDFE000; 
    vmmgr_switch_pml4_directory(pml4table);
    mmgr_syncwith_kernel();
    init_bump_addr();
    bump_start = bump_addr;
    //vmmgr_map_page_after_paging(0x9000, 0xFFFFFFFFFFA00000);
/*    for(i=0, frame=0x200000, virt=(0xFFFFFFFFFFA00000+(uint64_t)(&physbase)); i<PAGES_PER_TABLE; i++, frame=frame+PHY_PAGE_SIZE, virt+=VIRT_PAGE_SIZE){
      vmmgr_map_page_after_paging(frame, virt);
    }
    */
}
