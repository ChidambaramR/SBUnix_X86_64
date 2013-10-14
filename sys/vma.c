#include <sys/task.h>

static inline void vma_link_list(mm_struct *mm, vm_area_struct *vma, vm_area_struct *prev){
    if(prev){
        vma->vm_next = prev->vm_next;
        prev->vm_next = vma;
    }
    else{
        mm->mmap = vma;
        vma->vm_next = NULL;
    }
}


