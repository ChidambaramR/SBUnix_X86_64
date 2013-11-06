#include <defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mm/vmmgr_virtual.h>
#include <sys/mm/mmgr.h>
#include <sys/task.h>
#include <elf.h>
#include <sys/mmap.h>

#define UserCode  0x0000000000400000
#define UserData  0x0000000000600000
#define UserStack 0x0000000000bff000
#define VmaStart  0x0000000000400000

extern void switch_to_user();
extern task_struct* currentTask;
extern uint64_t get_cr3_register();
/*init_user_memory(){
    pte* new_page_table = get_new_page_table((pml4*)(currentTask->cr3 & 0xFFFFF000), user_code);
    
}*/

static __inline__ void Push(task_struct* task, uint64_t value)
{
    task->rsp -= 0x8;
    *((uint64_t *) task->rsp) = value;
}

void Init_Task(task_struct* task, uint16_t prio, const char* name, uint64_t code, uint64_t codeLen, uint64_t data, uint64_t dataLen, uint64_t stack, uint64_t entry_point){
    task->code_page = (void*)code;
    task->code_len = codeLen;
    task->data_page = (void*)data;
    task->data_len = dataLen;
    task->stack_page = (void*)stack;
    task->rsp = (uint64_t)(((uint64_t)task->stack_page) + VIRT_PAGE_SIZE );
    Push(task, 0);
    task->priority = prio;
    //task->userContext = 0;
    task->alive = TRUE;
    task->name = (char *)name;
    task->pid = alloc_pid();
    task->entry_point = (void*)entry_point;
}



static __inline__ void Push_General_Registers(task_struct* task){
    /*
     * Push initial values for saved general-purpose registers.
     * (The actual values are not important.)
     */
    Push(task, 0);  /* rax */
    Push(task, 0);  /* rbx */
    Push(task, 0);  /* rcx */
    Push(task, 0);  /* rdx */
    Push(task, 0);  /* rsi */
    Push(task, 0);  /* rdi */
    Push(task, 0);  /* rbp */
    Push(task, 0);  /* r15 */
    Push(task, 0);  /* r14 */
    Push(task, 0);  /* r13 */
    Push(task, 0);  /* r12 */
    Push(task, 0);  /* r11 */
    Push(task, 0);  /* r10 */
    Push(task, 0);  /* r09 */
    Push(task, 0);  /* r08 */
    Push(task, 0x23); /* ds */
    Push(task, 0x23); /* es */
    Push(task, 0x23); /* fs */
    Push(task, 0x23); /* gs */
    Push(task, 0x23); /* ss */
    Push(task, 0x1B); /* cs */
}


void create_new_task(){
}

uint32_t do_exec(/*char *name, char *environment*/){
   // uint64_t *data;
    uint64_t currentCode_page;
    uint64_t currentCode_len;
    uint64_t currentData_page;
    uint64_t currentData_len;
    uint64_t currentStack_page;
    uint64_t new_pte;
    task_struct* task;
  
   // uint32_t size;
    int codeLen, dataLen;
    uint64_t entry_point;
    char *codeBuf, *dataBuf;
   // struct posix_header_ustar executable;
   // char *prog_name = (char*)sub_malloc(strlen(name),1);
   // strcpy(prog_name, name);
    if( readelf(&codeBuf, &dataBuf, &codeLen, &dataLen, &entry_point) ){
        // Initialize code page
        task = (task_struct*)sub_malloc(sizeof(task_struct), 0);
        memset(task, 0, sizeof(task_struct));
        currentTask = task;
        currentTask->pgd = (pml4*)get_cr3_register();
        currentCode_page = UserCode;
        currentCode_len = codeLen;
        while(codeLen >= 0){
          mmap((void*)currentCode_page, currentCode_len, 0, 0, 0, 0);
          //new_pte = (uint64_t)mmgr_alloc_block();
          //vmmgr_map_page_after_paging(new_pte, currentCode_page, 1);
          memcpy((char*)(currentCode_page + (entry_point - UserCode)), (const char*)codeBuf, (uint32_t)codeLen);
          codeLen = codeLen - VIRT_PAGE_SIZE;
          currentCode_page += VIRT_PAGE_SIZE;
        }
        currentCode_page -= VIRT_PAGE_SIZE;
        
        // Initialize data page
        currentData_page = UserData;
        currentData_len = dataLen;
        while(dataLen >= 0){
          mmap((void*)currentData_page, currentData_len, 0, 0, 0, 0);
          //new_pte = (uint64_t)mmgr_alloc_block();
          //vmmgr_map_page_after_paging(new_pte, currentData_page, 1);
          memcpy((char*)currentData_page, (const char*)dataBuf, (uint32_t)dataLen);
          dataLen = dataLen - VIRT_PAGE_SIZE;
          currentData_page += VIRT_PAGE_SIZE;
        }
        currentData_page -= VIRT_PAGE_SIZE;

        // Initialize stack page
        currentStack_page = UserStack;
        new_pte = (uint64_t)mmgr_alloc_block();
        vmmgr_map_page_after_paging(new_pte, currentStack_page, 1);
        memset((void*)currentStack_page, 0, sizeof(VIRT_PAGE_SIZE));

        // Create the actual task structure
        Init_Task(task, TASK_PRIO_NORMAL, "first", currentCode_page, currentCode_len, currentData_page, currentData_len, currentStack_page, entry_point);
        switch_to_user();
        /*currentPage = UserCode;
        currentPage_size = usercode_len;
        init_user_memory();
        */
    }
    return 1;
}
