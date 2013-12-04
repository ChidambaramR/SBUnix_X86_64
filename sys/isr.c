#include <defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/kthread.h>
#include <sys/mm/mmgr.h>
#include <sys/idt.h>
#include <stdlib.h>
#include <sys/mmap.h>

extern kthread* currentThread;
extern kthread* ptable[100];
extern uint64_t get_cr3_register();
volatile int reading = 0;
volatile int reading_finished = 1;
char iobuff[1024];
char* io_buff;
extern Thr_Queue runQueue;
extern global_thread_list allThreadList;
extern void print_ls();
extern void print_ll();
extern void print_pwd();
extern int nextFreePid;
extern int tarfs_open(char*);
extern char files[][100];
extern int file_used[];
extern int do_cd(char*);

void reload_cr3(uint64_t pcr3){
    __asm__ __volatile__("movq %rdi, %cr3");
}

void sys_exit(){
  int pid;
  kthread* k_thread;
  /*
  Important: While coming to this function, the kernel actually executes in the context
  of the process. Thus we can easily get the PID of the process which currently issued the system call. 
  */
  pid = currentThread->pid;
  k_thread = ptable[pid];
  thread_cleanup(k_thread);
  ptable[pid] = NULL;
  free_pid();
  return;
}

void page_fault_handler(uint64_t err_code, void* err_ins){
   // A page fault has occurred.
   // The faulting address is stored in the CR2 register.
   uint64_t faulting_address, phys, fault_page;
   vm_area_struct *crawl;
   pt_entry* pte;
   asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
//   cls();
   pte  = get_pte_recurse(faulting_address);
    if(currentThread->kernel_thread == 0){
        crawl = currentThread->mmap;
        while(crawl){
          if(((uint64_t)crawl->vm_start - faulting_address) <= 100){
              if(currentThread->no_stack_pages <= 4){
              currentThread->no_stack_pages++;
              mmap((void*)((uint64_t)currentThread->mmap_cache->vm_start - 0x1000), VIRT_PAGE_SIZE, 0, 0, 0, 0, currentThread);
              }
              else{
              printf("Stack Overflow! Killing the process %d",currentThread->pid);
              currentThread->alive = 0;
//              while(1);
              remove_runnable_kthread(&runQueue, currentThread);
              remove_alllist_kthread(&allThreadList, currentThread);
              sys_exit();
              }
              return;   
          }               
          crawl = crawl->vm_next;
        } 
    }
  // printf("PFFFFPP %p \npte = %p \nCOW = %p \ncr3 = %x\n",*pte, pte, PTE_COW,get_cr3_register());
    if(*pte & PTE_COW){
        char buf[4096];
//        printf("wow COW\n");
        phys = (uint64_t)mmgr_alloc_block();        
//        printf("mapping phys %x to virt %x he he\n",phys,faulting_address);
        fault_page = PAGE_PHYSICAL_ADDRESS(&faulting_address);
//        printf("fault_page = %p he he",fault_page);
        memcpy(buf, (const char*)fault_page, 4096);
        vmmgr_map_page_after_paging((uint64_t)phys, (uint64_t)faulting_address, 1);
        reload_cr3(currentThread->pcr3);
//        *((uint64_t*)fault_page) = 2;
         memcpy((char*)fault_page, (const char*)buf, 4096);
//        printf("ha ha hai\n");
//        while(1);
        return;
    }
    else{
        printf("Segmentation fault. Killing the process %d",currentThread->pid);
        currentThread->alive = 0;
        remove_runnable_kthread(&runQueue, currentThread);
        remove_alllist_kthread(&allThreadList, currentThread);
        sys_exit();
        return;
    }
//   while(1);

   // The error code gives us details of what happened.
   int present = !(err_code & 0x1); // Page not present
   int rw = err_code & 0x2;           // Write operation?
   int us = err_code & 0x4;           // Processor was in user-mode?
   int reserved = err_code & 0x8;     // Overwritten CPU-reserved bits of page entry?
//   int id = err_code & 0x10;          // Caused by an instruction fetch?

   // Output an error message.
//   cls();
   printf("Page fault! ( ");
   if (present) {printf("Page Not Present ");}
   if (rw) {printf("read-only ");}
   if (us) {printf("user-mode ");}
   if (reserved) {printf("reserved ");}
   printf(") at 0x");
   printf("%p",faulting_address);
   printf("\n");
   printf("Faulting ins %p\n",((uint64_t)(err_ins)));
   PANIC(__FUNCTION__,__LINE__,"Page fault! ");
   while(1);
}

void general_protection_fault_handler(uint64_t err_code){
//  cls();
  PANIC(__FUNCTION__,__LINE__,"General Protection Fault! ");
}

void write(const char* str){
  printf("%s",str);
}

int sys_getpid(){
  return currentThread->pid;
}


void PushU(kthread* k_thread, uint64_t value){
  k_thread->krsp -= 0x8;
  *((uint64_t *) k_thread->krsp) = value; 
}

void fork_int(kthread* k_thread, regs* r){
  uint64_t pid = k_thread->pid;
  PushU(k_thread, (uint64_t)0x23); 
  PushU(k_thread, r->rsp); 
  PushU(k_thread, r->rflags); 
  PushU(k_thread, r->cs); 
  PushU(k_thread, r->rip); 
  PushU(k_thread, r->intNo); 
  PushU(k_thread, r->errCode); 
  PushU(k_thread, pid); // Pushing rax which is the value which fork should return 
  PushU(k_thread, r->rbx); 
  PushU(k_thread, r->rcx); 
  PushU(k_thread, r->rdx); 
  PushU(k_thread, r->rbp); 
  PushU(k_thread, r->rdi); 
  PushU(k_thread, r->rsi); 
  PushU(k_thread, r->r8); 
  PushU(k_thread, r->r9); 
  PushU(k_thread, r->r10); 
  PushU(k_thread, r->r11); 
  PushU(k_thread, r->r12); 
  PushU(k_thread, r->r13); 
  PushU(k_thread, r->r14); 
  PushU(k_thread, r->r15); 
  
}

int fork(regs *r){
  kthread* k_thread = (kthread*)sub_malloc(0,1);
  memcpy((char *)k_thread, (const char*)currentThread, sizeof(kthread));
  k_thread->pid = alloc_pid();
  ptable[k_thread->pid] = k_thread;
  k_thread->kstack = (void*)sub_malloc(1,1);
  k_thread->krsp = (((uint64_t) k_thread->kstack) + 2*VIRT_PAGE_SIZE -0x8); 
  k_thread->kstack = (void*)(((uint64_t) k_thread->kstack) + 2*VIRT_PAGE_SIZE - 0x8);
  k_thread->parent = currentThread;
  fork_int(k_thread, r);
  if ( !k_thread )
      PANIC(__FUNCTION__,__LINE__,"No mem");
  k_thread->cr3 = (uint64_t)sub_malloc(0, 1);
  k_thread->pcr3 = k_thread->cr3 - (uint64_t)0xFFFFFFFF80000000;
  clone_pgdir(currentThread->cr3, k_thread->cr3);
  copy_page_table(currentThread->cr3, k_thread->cr3, 1);
//  printf("%d",k_thread->cr3);
  add_to_joinQueue(currentThread, k_thread);
  alllist_kthread(k_thread);
  runnable_kthread(k_thread);
//  stackPage = (void*)UserStack;
  return 0;  
}

void sleep(uint64_t time){
  currentThread->sleeping = time;
}

signed int doread(char* buf,int fd){
  if(fd == STDIN){
  __asm__ __volatile__("sti");
  //printf("querying for read_busy by %d val = %d\n",currentThread->pid, reading);
  if(reading == 1 || reading_finished == 0){
//      printf("returning -1 to %d",currentThread->pid);
      return -1;
  }
//  cls();
//  printf("setting read busy to 1 b %d\n",currentThread->pid);
  reading = 1;
  reading_finished = 0;
  io_buff = iobuff;
  while(reading == 1);
  //printf("%s io buff \n",iobuff);
  memcpy(buf,iobuff, strlen(iobuff));
  reading = 0;
  reading_finished = 1;
  //printf("%d has finieshed reading\n",currentThread->pid);
  return(strlen(buf));
  }
  else{
  // File code
  if(file_used[fd] == 1){
    void *src = tarfs_read(files[fd]);
    memcpy(buf, (char*)src, 100);
    return 0;
  }
  else{
    printf("File is already open by another process\n");
    return -1;
  }
  }
}

void temp(){
  return; 
}
void wait(){
  __asm__ __volatile__("sti");
  while(1){
//  printf("find next\n");
  temp();
//  a++;
  volatile kthread* next = (volatile kthread*)currentThread->head.next;
  if(next != NULL)
      continue;
  else
      break;
  }
  return;
}

int do_execve(char* name){
void* start = tarfs_read(name);
printf("in do_execve\n");
if( !start ){
  //  printf("hmmm\n");
    return -1;
}
main_execve(name);
printf("iffn do_execve\n");
disable_interrupts();
runnable_kthread(currentThread);
enable_interrupts();
printf("iffggn do_execve\n");
Schedule();
printf("returning hereee\n");
//while(1);
return 0;
}

void print_process(){
    kthread* k_thread;
    uint16_t i;
    char* mode;
    printf("Name PID Mode\n");
    for(i=2; i < nextFreePid ; i++){
    //for(i=2; i<3; i++){
//      printf("%p\n",ptable[i]);
      k_thread = ptable[i];
      if(k_thread->kernel_thread == 1)
          mode = "Kernel";
      else
          mode = "User";
        printf("%s %d %s %d\n",k_thread->name, k_thread->pid, mode, nextFreePid);
    }
}

void do_cls(){
  cls();
  return;
}

void do_ls(){
  print_ls();
  return;
}

void do_ll(){
  print_ll();
  return;
}

int do_sbrk(int pr){
  if(pr != 0){
      currentThread->brk = currentThread->brk + pr;
  }
  return currentThread->brk;
}

int do_open(char* name){
  int fd;
  fd = tarfs_open(name);
  return fd;
}

void do_close(int fd){
  file_used[fd]=0;
  return;
}

int changed(char* name){
  int ret;
  ret = do_cd(name);
  return ret;
}

void do_pwd(){
  print_pwd();
  return;
}
