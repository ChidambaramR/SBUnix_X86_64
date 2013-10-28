/*init_user_memory(){
    pte* new_page_table = get_new_page_table((pml4*)(currentTask->cr3 & 0xFFFFF000), user_code);
    
}

uint32_t do_exec(char *name, char *environment){
    uint32_t codeseg_len, dataseg_len;
    uint64_t *data;
    uint64_t currentPage;
    uint32_t size;
    struct posix_header_ustar executable;
    char *prog_name = (char*)sub_malloc(strlen(name),1);
    strcpy(prog_name, name);
    readfelf(&executable);
    if(executable){
        currentPage = UserCode;
        currentPage_size = usercode_len;
        
        
        init_user_memory();
        
    }
}
*/
