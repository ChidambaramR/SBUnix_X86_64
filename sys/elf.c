#include <sys/tarfs.h>
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mm/mmgr.h>
#include <sys/mm/vmmgr_virtual.h>
#include <defs.h>

char cwd[] = "rootfs/";
char files[50][100];
int file_used[50]; // A value of 1 in fd means, file is used

int file_count=1; // Start with 1 as FD 0 is to indicate error

struct fs_tree{
  char name[50];
  struct fs_tree* child[30];
  int no_child;
};

struct fs_tree rootfs;
struct fs_tree *pwd;

void printSectHrds(void* bin_start, Elf64_Ehdr* bin_elf_start, uint16_t idx){
        Elf64_Shdr* sectHdr;
            sectHdr = (Elf64_Shdr*)((bin_start + bin_elf_start->e_shoff + idx*(sizeof(Elf64_Shdr)) + sizeof(Elf64_Ehdr)));
            printf("Sec index %d Sec name offset = %d Sec type = %d Sec attributes %d Sec virtual address %x Sec offset %d Sec size %d Sec link %d Sec info %d Sec align = %d Sec entSize = %d\n",idx,sectHdr->sh_name,sectHdr->sh_type,sectHdr->sh_flags,sectHdr->sh_addr,sectHdr->sh_offset,sectHdr->sh_size,sectHdr->sh_link,sectHdr->sh_info,sectHdr->sh_addralign,sectHdr->sh_entsize);

}

void printElfHdr(void *bin_start, Elf64_Ehdr* bin_elf_start){
      printf("Elf_magic %s Elf_type %d Elf_mach_type %d elf_version %d elf_entry_addr %x elf_pf_offset %d elf_sh_offset %d elf_flags %d, elf_size %d, elf_ph_size %d, elf_ph_num %di, elf_sh_size %d, elf_sh_size %d, elf_sh_num %d, elf_str_index %d\n",bin_elf_start->e_ident,bin_elf_start->e_type,bin_elf_start->e_machine,bin_elf_start->e_version,bin_elf_start->e_entry,bin_elf_start->e_phoff,bin_elf_start->e_shoff,bin_elf_start->e_flags,bin_elf_start->e_ehsize,bin_elf_start->e_phentsize,bin_elf_start->e_phnum,bin_elf_start->e_shentsize,bin_elf_start->e_shnum,bin_elf_start->e_shstrndx);
}

void printPgmHdr(void *bin_start, Elf64_Ehdr* bin_elf_start, uint16_t idx){
        Elf64_Phdr* pgmHdr;
        pgmHdr = (Elf64_Phdr*)((bin_start + sizeof(Elf64_Ehdr) +  idx*sizeof(Elf64_Phdr)));
        printf("Pgm_vaddr %x, Pgm_offset = %d, Pgm_size = %d\n", pgmHdr->p_vaddr, pgmHdr->p_offset, pgmHdr->p_filesz);
}

int round(int no){
  no = (((no/512)+1)* 512);
  return no;
}

struct fs_tree* get_pwd(char* name){
    int pchild=0, i;
    struct fs_tree *crawl = pwd;
    pchild = crawl->no_child;
    if(crawl == &rootfs)
      pchild = pchild - 1;
    for(i=0; i<pchild; i++){
      if(strcmp(crawl->child[i]->name, name) == 0)
        break;
      //printf("%s \n",crawl->child[i]->name);
    }
    return crawl->child[i];
}

void print_pwd(){
    printf("%s\n",pwd->name);
    return;
}

void tarfs_dir(){
        uint64_t end = (uint64_t)&(_binary_tarfs_end);
        struct posix_header_ustar *header;
        header = (struct posix_header_ustar*)(&_binary_tarfs_start);
         while( (uint64_t)header <= end ){
           if(my_atool(header->size) > 0){
                strncpy(files[file_count++], (char*)header->name, strlen((char*)header->name));
            }
        if(my_atool(header->size) == 0)
            header = header + 1 + (my_atool(header->size));
        else
            header = (struct posix_header_ustar*)((uint64_t)(header + 1) + (uint64_t)round(my_atool(header->size)));
           }
        return;
}

bool strncmp(char *src, char *dst, int n){
  int i=0;
  for(i=0; i<n; i++)
    if(src[i] == dst[i])
      continue;
    else
      return TRUE;
    return FALSE;
}

void init_tarfs(){
        uint64_t end = (uint64_t)&(_binary_tarfs_end);
        struct posix_header_ustar *header;
        pwd = &rootfs;
        struct fs_tree* temp_rt = &rootfs;
        struct fs_tree* old_dir = &rootfs;

        strncpy(rootfs.name, "rootfs/", 7);  
        header = (struct posix_header_ustar*)(&_binary_tarfs_start);
         while( (uint64_t)header <= end ){
          if(strncmp(old_dir->name, (char*)header->name, strlen((char*)old_dir->name)) == 0)
              temp_rt = old_dir;
          else
              temp_rt = &rootfs;
          if(my_atool(header->size) > 0){
              struct fs_tree* temp = (struct fs_tree*)sub_malloc(sizeof(struct fs_tree),0);
              memset(temp, '\0', sizeof(struct fs_tree));
              strncpy(temp->name, (char*)header->name, strlen((char*)header->name));
              temp_rt->child[(temp_rt->no_child)++] = temp;
          }
          else{
              struct fs_tree* temp = (struct fs_tree*)sub_malloc(sizeof(struct fs_tree),0);
              memset(temp, '\0', sizeof(struct fs_tree));
              strncpy(temp->name, (char*)header->name, strlen((char*)header->name));
              temp_rt->child[temp_rt->no_child] = temp;
              (temp_rt->no_child)++;
              old_dir = temp;
          }
        if(my_atool(header->size) == 0)
            header = header + 1 + (my_atool(header->size));
        else
            header = (struct posix_header_ustar*)((uint64_t)(header + 1) + (uint64_t)round(my_atool(header->size)));
           }
        return;
 
}
int tarfs_open(char* name){
        int i=0;
        int fd=0;
        for(i=1; i<file_count; i++){
            if(strcmp(name, files[i]) == 0){
                fd=i;
                break;
            }
        }   
        if(fd != 0)
          file_used[fd] = 1;
        return fd;
}
void* tarfs_read(char* name){
//        uint64_t start = (uint64_t)&(_binary_tarfs_start);
        void* start_addr = NULL;
        int found = 0;
        uint64_t end = (uint64_t)&(_binary_tarfs_end);
        struct posix_header_ustar *header;
        header = (struct posix_header_ustar*)(&_binary_tarfs_start);
        //`printf("tarfs read\n");
         while( (uint64_t)header <= end ){
           if(strcmp((char*)header->name, name) == 0){
//              printf("elf file found");
              found = 1;
              start_addr = (void*)(header + 1);
              break;
           } 
        if(my_atool(header->size) == 0)
            header = header + 1 + (my_atool(header->size));
        else
            header = (struct posix_header_ustar*)((uint64_t)(header + 1) + (uint64_t)round(my_atool(header->size)));
        }
  //      printf("Returning here\n");
        if(found)
          return start_addr;
        else
          return (void*)NULL;
}

int do_cd(char* name){
  printf("received cd %s",name);
  if(strcmp(name, "home") == 0){
    pwd = &rootfs;
    return 0;
  }
  printf("searching in trafs\n");
  void* addr = tarfs_read(name);
  if(!addr)
    return -1;
  
  pwd = get_pwd(name);
  //printf("pwd %s\n",pwd->name);
  return 0;
}

void print_ls_rec(struct fs_tree *crawl){
    if(!crawl)
      return;
    int pchild=0, i;
    pchild = crawl->no_child;
    if(crawl == &rootfs)
      pchild -= 1;
    for(i=0; i<pchild; i++){
      printf("%s ",crawl->child[i]->name);
      //print_ls_rec(crawl->child[i]);
    }
    //print_ls(
     
}

void print_ls(){
  print_ls_rec(pwd);
  printf("\n");
  return;
}

void print_ll(){

    uint64_t end = (uint64_t)&(_binary_tarfs_end);
    struct posix_header_ustar *header;
    header = (struct posix_header_ustar*)(&_binary_tarfs_start);
        //`printf("tarfs read\n");
    while( (uint64_t)header <= end ){
      printf("%s ",(char*)header->name);
      if(my_atool(header->size) == 0)
          header = header + 1 + (my_atool(header->size));
      else
          header = (struct posix_header_ustar*)((uint64_t)(header + 1) + (uint64_t)round(my_atool(header->size)));
    }
    
}
/*
Structure of ELF File.
Start of 3*512 is the Elf header.
Immediately following the Elf header is the program header.
After the program header ends, we get the actual contents of the program sections.
From the start, elf_header + section offset we get the section headers
*/
uint16_t readelf(char *name, struct exec *executable, uint16_t *pgm_entries, uint64_t *entry_point){
        Elf64_Ehdr *bin_elf_start;
        Elf64_Shdr *sectHdr;
        Elf64_Phdr *pgmHdr;
        uint16_t i;
        //uint64_t ans[100];
        uint64_t load_size, load_start, load_addr;
        uint64_t length = 0;
        uint64_t text_start;
        //char *user_code, *data_code, *code_buf, *data_buf;
        uint64_t actual_mem_start;
        void* bin_start;
        void* load_buf;
        //bin_elf_start = (Elf64_Ehdr*)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar));
        bin_start = (void*)tarfs_read(name);
        if((uint64_t)bin_start == NULL)
            return 0;
        
        bin_elf_start = (Elf64_Ehdr*)bin_start;
        *entry_point = bin_elf_start->e_entry;
        *pgm_entries = bin_elf_start->e_phnum - 1;
        //bin_start = (void*)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar));
        //printElfHdr(bin_start, bin_elf_start);
        //printPgmHdr(bin_start, bin_elf_start, 0);
        sectHdr = (Elf64_Shdr*)((bin_start + bin_elf_start->e_shoff + sizeof(Elf64_Ehdr)));
        text_start = sectHdr->sh_addr;
        //sectHdr = (Elf64_Shdr*)((bin_start + bin_elf_start->e_shoff + sizeof(Elf64_Ehdr) + sizeof(Elf64_Shdr)));
        actual_mem_start = (uint64_t)((uint64_t)bin_start + sizeof(Elf64_Ehdr) + ((*pgm_entries))*sizeof(Elf64_Phdr) + bin_elf_start->e_phoff);
        for(i=0; i < (*pgm_entries); i++){
          pgmHdr = (Elf64_Phdr*)((bin_start + sizeof(Elf64_Ehdr) +  i*sizeof(Elf64_Phdr)));
          load_size = pgmHdr->p_memsz;
          load_start = pgmHdr->p_vaddr - pgmHdr->p_offset;
          if(i==0)
            load_addr = text_start;
          else
            load_addr = pgmHdr->p_vaddr;
          if(i==0)
            load_buf = (void*)(actual_mem_start + length);
          else
            load_buf = (void*)((uint64_t)bin_elf_start + pgmHdr->p_offset);
          length += load_size;
          executable[i].seg_length = load_size;
          executable[i].seg_page_start = load_start;
          executable[i].seg_actual_start = load_addr;
          executable[i].seg_mem = (void *)load_buf;
        }
        /*printf("%d",ans[0]);
        *user_data_length = sectHdr->sh_size;
        code_buf = sub_malloc(*user_code_length,0);
        if(!code_buf)
          return 0;
        memset(code_buf, 0, sizeof(code_buf));
        data_buf = sub_malloc(*user_data_length,0);
        if(!data_buf)
          return 0;
        memset(data_buf, 0, sizeof(data_buf));
        //printf("section name %d\n", sectHdr->sh_size);
        user_code = (char*)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar) + sizeof(Elf64_Ehdr) + 2*sizeof(Elf64_Phdr) + bin_elf_start->e_phoff);
        memcpy(code_buf, user_code, *user_code_length);
        data_code = (char*)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar) + sizeof(Elf64_Ehdr) + 2*sizeof(Elf64_Phdr) + bin_elf_start->e_phoff + *user_code_length);
        memcpy(data_buf, data_code, *user_data_length);
        *codeBuf = code_buf;
        *dataBuf = data_buf;
        */
        return 1;
}

