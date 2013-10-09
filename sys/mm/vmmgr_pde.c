#include <sys/mm/vmmgr_pde.h>

inline void pd_entry_add_attrib(pd_entry *e, uint64_t attrib){
    *e |= attrib;
}

inline void pd_entry_del_attrib(pd_entry *e, uint64_t attrib){
    *e &= (~attrib);
}

/*
Sets the 32 bit base address of the page table. Since it is a physical
page again it, is aligned in terms of 4KB which means the lowest three
bits are always 0. So we need not left shift the page. 
*/
inline void pd_entry_set_frame(pd_entry *e, uint64_t addr){
    *e = (*e & ~PDE_FRAME) | addr;
}

inline bool pd_entry_is_present(pd_entry e){
    return ( e & PDE_PRESENT );
}

inline bool pd_entry_is_writable(pd_entry e){
    return ( e & PDE_WRITABLE );
}

inline uint32_t pd_entry_pfn(pd_entry e){
    return ( e & PDE_FRAME );
}

inline bool pd_entry_is_user (pd_entry e) {
  return e & PDE_USER;
}

inline bool pd_entry_is_4mb (pd_entry e) {
  return e & PDE_4MB;
}
