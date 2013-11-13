#include <sys/tarfs.h>
#include <elf.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mm/mmgr.h>
#include <sys/mm/vmmgr_virtual.h>
#include <defs.h>

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

/*
Structure of ELF File.
Start of 3*512 is the Elf header.
Immediately following the Elf header is the program header.
After the program header ends, we get the actual contents of the program sections.
From the start, elf_header + section offset we get the section headers
*/
uint16_t readelf(struct exec *executable, uint16_t *pgm_entries, uint64_t *entry_point){
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
        bin_elf_start = (Elf64_Ehdr*)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar));
        *entry_point = bin_elf_start->e_entry;
        *pgm_entries = bin_elf_start->e_phnum - 1;
        bin_start = (void*)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar));
        //printElfHdr(bin_start, bin_elf_start);
        //printPgmHdr(bin_start, bin_elf_start, 0);
        sectHdr = (Elf64_Shdr*)((bin_start + bin_elf_start->e_shoff + sizeof(Elf64_Ehdr)));
        text_start = sectHdr->sh_addr;
        //sectHdr = (Elf64_Shdr*)((bin_start + bin_elf_start->e_shoff + sizeof(Elf64_Ehdr) + sizeof(Elf64_Shdr)));
        actual_mem_start = (uint64_t)(&(_binary_tarfs_start) + 3*sizeof(struct posix_header_ustar) + sizeof(Elf64_Ehdr) + (*pgm_entries)*sizeof(Elf64_Phdr) + bin_elf_start->e_phoff);
        for(i=0; i < (*pgm_entries); i++){
          pgmHdr = (Elf64_Phdr*)((bin_start + sizeof(Elf64_Ehdr) +  i*sizeof(Elf64_Phdr)));
          load_size = pgmHdr->p_memsz;
          load_start = pgmHdr->p_vaddr - pgmHdr->p_offset;
          if(i==0)
            load_addr = text_start;
          else
            load_addr = pgmHdr->p_vaddr;
          load_buf = (void*)(actual_mem_start + length);
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

