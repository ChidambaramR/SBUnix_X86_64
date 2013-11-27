

#include <defs.h>

typedef uint64_t pt_entry;

enum PAGE_PTE_FLAGS{
    
    PTE_PRESENT = 1,                  //Is page present in physical memory.
    PTE_WRITABLE = 2,                 //Is page write permitted. 1 means both read and write access
    PTE_USER = 4,                     /*Is it a user page or kernel page. 1 means both user and 
                                        supervisor can read*/
    PTE_WRITETHROUGH = 8,             /*Write back or write through cache policy for the 
                                        page or page translation table to which this entry points.
                                        means writethrough cache */
    PTE_NOT_CACHABLE = 0x10,          /*Whether the page or pagen translation table this entry
                                        points to is cachable. 0 means cahcable*/
    PTE_ACCESSED = 0x20,              /*Whether the page or pagen translation table this entry
                                        points to is accessed */
    PTE_DIRTY = 0x40,                 /*Whether the physical page pointed to by this entry is written.
                                        Software must clear this bit*/
    PTE_PAT = 0x80, 
    PTE_GLOBAL = 0x100,               /*Value 1 indicates it is a global page. TLB for a global page
                                        is not invalidated when CR3 is modified.*/
    PTE_AVL = 0x700,                  //Bits 9,10 and 11
    PTE_COW = 0x4000000000000000,
    PTE_FRAME = 0x00000FFFFFFFF000    // Target physical address which is directly stored.
};

//! sets a flag in the page table entry
extern void pt_entry_add_attrib(pt_entry* e, uint64_t attrib);

//! clears a flag in the page table entry
extern void pt_entry_del_attrib(pt_entry* e, uint64_t attrib);

//! sets a frame to page table entry
extern void pt_entry_set_frame(pt_entry*, uint64_t);

//! test if page is present
extern bool pt_entry_is_present(pt_entry e);

//! test if page is writable
extern bool pt_entry_is_writable(pt_entry e);

//! get page table entry frame address
extern uint32_t pt_entry_pfn(pt_entry e);

extern bool pt_entry_is_accessed(pt_entry e);
