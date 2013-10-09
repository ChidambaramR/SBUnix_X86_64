#include <sys/mm/vmmgr_pte.h>

inline void pt_entry_add_attrib(pt_entry *e, uint64_t attrib){
    *e |= attrib;
}

inline void pt_entry_del_attrib(pt_entry *e, uint64_t attrib){
    *e &= ~attrib;
}

inline void pt_entry_set_frame(pt_entry *e, uint64_t addr){
    *e = ((*e & ~PTE_FRAME) | addr);
}

inline bool pt_entry_is_present(pt_entry e){
    return (e & PTE_PRESENT);
}

inline bool pt_entry_is_writable(pt_entry e){
    return (e & PTE_WRITABLE);
}

inline uint32_t pt_entry_pfn(pt_entry e){
    return (e & PTE_FRAME);
}

inline bool pt_entry_is_accessed(pt_entry e){
    return (e & PTE_ACCESSED);
}

inline bool pt_entry_is_user(pt_entry e){
    return (e & PTE_USER);
}

inline bool pt_entry_is_dirty(pt_entry e){
    return (e & PTE_DIRTY);
}
/*
We dont need to check if it is a user page or not as the page directory entry
pointing to this page tells whether this block is user's or not.
*/

