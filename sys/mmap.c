#include <stdio.h>
#include <defs.h>
#include <sys/mmap.h>
#include <sys/task.h>
#include <stdlib.h>
#include <sys/mm/mmgr.h>
#include <sys/kthread.h>
#include <sys/mm/vmmgr_virtual.h>


void insert_vma(vm_area_struct *vma, kthread* k_thread){
//  vm_area_struct *last = k_thread->mmap_cache;
  vm_area_struct *first = k_thread->mmap;
  vma->vm_next = NULL;
  if(!first){
      k_thread->mmap = vma;
  }
  else{
      k_thread->mmap_cache->vm_next = vma;
  }
      k_thread->mmap_cache = vma;
}

void mmap(void *addr, uint32_t length, int prot, int flags, int fd, uint64_t offset, kthread* k_thread){
          uint64_t new_pte;
          vm_area_struct *vma = (vm_area_struct*)sub_malloc(sizeof(vm_area_struct),0);
          memset(vma, 0, sizeof(vm_area_struct));
          new_pte = (uint64_t)mmgr_alloc_block();
          vmmgr_map_page_after_paging(new_pte, (uint64_t)addr, 1);
          vma->name = "Code section";
          vma->vm_start = addr;
          vma->vm_end = (void*)((uint64_t)vma->vm_start + (uint64_t)0x1000);
          insert_vma(vma, k_thread);

}
