#include <libtitan.h>
#include <vfs.h>
#include <vmm.h>
#include <scheduler.h>


#define PT_LOAD 1
// Cheatsheet: https://gist.github.com/x0nu11byt3/bcb35c3de461e5fb66173071a2379779

typedef struct{
  unsigned char eident[16];
  uint16_t e_type;
  uint16_t e_machine;
  uint32_t e_version;
  uint64_t e_entry;
  uint64_t e_phoff;
  uint64_t e_shoff;
  uint32_t e_flags;
  uint16_t e_ehsize;
  uint16_t e_phentsize;
  uint16_t e_phnum;
  uint16_t e_shentsize;
  uint16_t e_shnum;
  uint16_t e_shstrndx;
} elf64_ehdr_t;

typedef struct {
  uint32_t p_type;
  uint32_t p_flags;
  uint64_t p_offset;
  uint64_t p_vaddr;
  uint64_t p_paddr;
  uint64_t p_filesz;
  uint64_t p_memsz;
  uint64_t p_align;
} elf64_phdr_t;

typedef struct{
  uint32_t sh_name;
  uint32_t sh_type;
  uint64_t sh_flags;
  uint64_t sh_addr;
  uint64_t sh_offset;
  uint64_t sh_size;
  uint32_t sh_link;
  uint32_t sh_info;
  uint64_t sh_addralign;
  uint64_t sh_entsize;
} elf64_shdr_t;

void dump_ehdr(elf64_ehdr_t ehdr){
  printf("ELF Signature: %s\n", ehdr.eident);
  printf("ELF phoff: %lu\n", ehdr.e_phoff);
  printf("ELF phnum: %d\n", (int) ehdr.e_phnum);
}

void dump_phdr(){

}

// Returns the start address
void* load_elf(const char* filename){
  elf64_ehdr_t ehdr;
  
  // Getting the elf from the tarball
  int handle = vfs_open(filename, 0);
  vfs_read(handle, (void*) &ehdr, sizeof(elf64_ehdr_t));
  
  dump_ehdr(ehdr);

  elf64_phdr_t phdr;
  vfs_seek(handle, ehdr.e_phoff);
  size_t file_pos = ehdr.e_phoff;

  for(int i=0; i<ehdr.e_phnum; i++){
    vfs_read(handle, (void*) &phdr, sizeof(elf64_phdr_t));
    file_pos += sizeof(elf64_phdr_t);
    if(phdr.p_type != PT_LOAD){
      continue;
    }
    if(!vmm_alloc(phdr.p_vaddr, phdr.p_memsz, VM_FLAG_READ_WRITE | VM_FLAG_USER, NULL)){
      PANIC("exec_elf", "failed to allocate section");
    }

    vfs_seek(handle, phdr.p_offset);
    vfs_read(handle, (void*) phdr.p_vaddr, phdr.p_filesz); 
    vfs_seek(handle, file_pos);
  }

  return (void*) ehdr.e_entry;
}


void exec_elf(const char* filename){
  vmm_t vmm = create_vmm();
  switch_vm_space(&vmm);

  create_proc(load_elf(filename), (void*) filename, vmm, true, true);

  switch_vm_space(&current_process->vmm);
}
