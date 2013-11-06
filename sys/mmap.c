#include <stdio.h>
#include <defs.h>
#include <sys/mmap.h>
#include <sys/task.h>
#include <stdlib.h>
#include <sys/mm/mmgr.h>
#include <sys/mm/vmmgr_virtual.h>

extern task_struct *currentTask;

void insert_vma(vm_area_struct *vma){
  vm_area_struct *last = currentTask->mmap_cache;
  vm_area_struct *first = currentTask->mmap;
  if(!first)
      currentTask->mmap = vma;    
  vma->vm_next = NULL;
  //current->mmap_cache = vma;
  if(last == NULL){
      currentTask->mmap_cache = vma;
  }
  else{
      last->vm_next = vma;
      currentTask->mmap_cache = vma;
  }
}

void mmap(void *addr, uint32_t length, int prot, int flags, int fd, uint64_t offset){
          uint64_t new_pte;
          vm_area_struct *vma = (vm_area_struct*)sub_malloc(sizeof(vm_area_struct),0);
          memset(vma, 0, sizeof(vm_area_struct));
          new_pte = (uint64_t)mmgr_alloc_block();
          vmmgr_map_page_after_paging(new_pte, (uint64_t)addr, 1);
          vma->name = "Code section";
          vma->vm_start = addr;
          vma->vm_end = (void*)((uint64_t)vma->vm_start + (uint64_t)0x1000);
          insert_vma(vma);

}
