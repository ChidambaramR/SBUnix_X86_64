#include <sys/mm/vmmgr_pml4.h>
#include <defs.h>

inline void pml4e_entry_add_attrib(pml4e_entry *e, uint64_t attrib){
    *e |= attrib;
}

inline void pml4e_entry_del_attrib(pml4e_entry *e, uint64_t attrib){
    *e &= (~attrib);
}

/*
Sets the 32 bit base address of the page table. Since it is a physical
page again it, is aligned in terms of 4KB which means the lowest three
bits are always 0. So we need not left shift the page. 
*/
inline void pml4e_entry_set_frame(pml4e_entry *e, uint64_t addr){
    *e = (*e & ~PML4E_FRAME) | addr;
}

inline bool pml4e_entry_is_present(pml4e_entry e){
    return ( e & PML4E_PRESENT );
}

inline bool pml4e_entry_is_writable(pml4e_entry e){
    return ( e & PML4E_WRITABLE );
}

inline uint32_t pml4e_entry_pfn(pml4e_entry e){
    return ( e & PML4E_FRAME );
}

inline bool pml4e_entry_is_user (pml4e_entry e) {
  return e & PML4E_USER;
}

inline bool pml4e_entry_is_4mb (pml4e_entry e) {
  return e & PML4E_4MB;
}


