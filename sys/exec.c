#include <defs.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mm/vmmgr_virtual.h>
#include <sys/mm/mmgr.h>
#include <elf.h>

#define UserCode 0x400000
#define UserData 0x600000

/*init_user_memory(){
    pte* new_page_table = get_new_page_table((pml4*)(currentTask->cr3 & 0xFFFFF000), user_code);
    
}*/

uint32_t do_exec(/*char *name, char *environment*/){
   // uint64_t *data;
    uint64_t currentCode_page;
    uint64_t currentData_page;
    uint64_t new_pte;
  
   // uint32_t size;
    int codeLen, dataLen;
    char *codeBuf, *dataBuf;
   // struct posix_header_ustar executable;
   // char *prog_name = (char*)sub_malloc(strlen(name),1);
   // strcpy(prog_name, name);
    if( readelf(&codeBuf, &dataBuf, &codeLen, &dataLen) ){
        currentCode_page = UserCode;
        while(codeLen >= 0){
          new_pte = (uint64_t)mmgr_alloc_block();
          vmmgr_map_page_after_paging(new_pte, currentCode_page);
          memcpy((char*)currentCode_page, (const char*)codeBuf, (uint32_t)codeLen);
          codeLen = codeLen - VIRT_PAGE_SIZE;
          currentCode_page += VIRT_PAGE_SIZE;
        }
        currentData_page = UserData;
        while(dataLen >= 0){
          new_pte = (uint64_t)mmgr_alloc_block();
          vmmgr_map_page_after_paging(new_pte, currentData_page);
          memcpy((char*)currentData_page, (const char*)dataBuf, (uint32_t)dataLen);
          dataLen = dataLen - VIRT_PAGE_SIZE;
          currentData_page += VIRT_PAGE_SIZE;
        }
        /*currentPage = UserCode;
        currentPage_size = usercode_len;
        init_user_memory();
        */
    }
    return 1;
}
