
#ifndef _TASK_H
#define _TASK_H
#include <defs.h>
#include <sys/mm/vmmgr_virtual.h>
/*
#include <sys/list.h>

*/

/*
The kernel represents a process's address space with a data structure called the
memory descriptor. This structure contains all the information related to the process
address space.
*/

/*
typedef uint64_t pgprotval_t;

typedef struct pgprot {
  pgprotval_t pgprot;
} pgprot_t;

typedef enum{
TASK_PRIO_NORMAL = 1,
TASK_PRIO_HIGH = 2,
} task_priority;
*/
struct vmas {
  char* name;
  void *vm_start, *vm_end; // Start and end of the region
  uint16_t vm_flags;
  struct vmas *vm_next;
  //pgprot_t vm_page_prot; // protection attributes for this region
  //vm_operations_struct *vm_ops;
};
typedef struct vmas vm_area_struct;

/*
typedef struct registers_t{
    //uint64_t ds;
  uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rsi, rdi, rbp, rdx, rcx, rbx, rax;
  uint64_t rip, cs, rflags, rsp, ss;
} regs_t;


typedef int __kernel_pid_t;
typedef __kernel_pid_t pid_t;

typedef struct _trapFrame {
   uint64_t rsp;
   uint64_t rbp;
   uint64_t rip;
   uint64_t rdi;
   uint64_t rsi;
   uint64_t rax;
   uint64_t rbx;
   uint64_t rcx;
   uint64_t rdx;
   uint64_t rflags;
   uint64_t r8;
   uint64_t r9;
   uint64_t r10;
   uint64_t r11;
   uint64_t r12;
   uint64_t r13;
   uint64_t r14;
   uint64_t r15;
  */ /*
      note: we can add more registers to this.
      For a complete trap frame, you should add:
        -Debug registers
        -Segment registers
        -Error condition [if any]
        -v86 mode segment registers [if used]
   *//*
}trapFrame;

typedef struct ts task_struct;

struct ti {
    //struct exec_domain      *exec_domain;   // execution domain 
    uint16_t  flags;          // low level flags 
    uint16_t  status;         / thread synchronous flags 
    uint16_t  preempt_count;  // 0 => preemptable, <0 => BUG 
    task_struct *task;          // main task structure 
};
typedef struct ti thread_info;

struct ts{
     // Dont modify the 1st two elements. Its offset is hardcoded in task_switch.s file
    uint64_t rsp;
    void* entry_point;
    void* code_page;
    void* data_page;
    void* stack_page;
    uint16_t code_len;
    uint16_t data_len;
    uint16_t priority;
    void* stackLimit;
    uint64_t start_code, end_code, start_data, end_data;
    uint64_t start_brk, brk, start_stack, start_mmap;
    uint64_t arg_start, arg_end, env_start, env_end;
    vm_area_struct *mmap; // Pointer to first region desc
    vm_area_struct *mmap_cache; // Pointer to last region desc
    bool alive;
    pid_t pid;
    char *name;
    pml4* pgd;
    struct ts* nextTask, *prevTask;
    struct ts* nextRunTask, *prevRunTask;
    uint64_t cr3;
    regs_t regs;
    uint16_t state;
    uint16_t flags;
    uint16_t prio, static_prio;
    uint16_t exit_code;
    thread_info *th_info;
        
};
typedef struct ts task_struct;

struct All_TaskList{
    task_struct *head, *tail;
};
typedef struct All_TaskList TaskList;

struct Run_TaskList{
    task_struct *head, *tail;
};
typedef struct Run_TaskList runQueue_List;

void insert_TaskList(TaskList*, task_struct*);
void append_TaskList(TaskList*, task_struct*);
void remove_TaskList(TaskList*, task_struct*);
void insert_runQueue_Task(runQueue_List*, task_struct*);
void append_runQueue_Task(runQueue_List*, task_struct*);
void remove_runQueue_Task(runQueue_List*, task_struct*);

int alloc_pid();

*/
#endif


