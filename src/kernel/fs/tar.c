#include <multiboot2.h>
#include <vmm.h>
#include <mem.h>
#include <str.h>
#include <tar.h>
#include <vfs.h>

typedef struct {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
} tar_header_t;

#define HEADER_LIST_SIZE 5
tar_header_t* file_headers[HEADER_LIST_SIZE];

static size_t get_file_size(const char* in){
    size_t size = 0;
    size_t j;
    size_t count = 1;

    for (j = 11; j > 0; j--, count *= 8)
        size += ((in[j - 1] - '0') * count);

    return size;

}


static void parse_tar_file(tar_header_t* header){
  uintptr_t header_addr = (uintptr_t) header;
  int i = 0;
  while(*((char*) (&header->filename)) != '\0'){
    printf("tar: found file %s\n", header->filename); 
    size_t file_size = get_file_size(header->size);

    // Next file is located on 512 byte boundry
    header_addr += ((file_size/ 512) + 1) * 512;
    
    //TODO: Just use ALIGN_UP 
    if(file_size % 512){
      header_addr += 512;
    }
    
    file_headers[i] = header;
    header = (tar_header_t*) header_addr;
    i++;
  } 
}


static int get_file_header(const char* file_name){
  for(int i=0; i<HEADER_LIST_SIZE; i++){
    if(strncmp(file_headers[i]->filename, file_name, 100) == 0){
      return i; 
    }
  }

  return -1;
}

file_descriptor_t ustar_open(const char* file_name, int flags){
  int file_handle = get_file_header(file_name);
  if(file_handle == -1){
    ERROR("tar", "file not found");
  }
  tar_header_t* file_header = file_headers[file_handle];
  file_descriptor_t fd = {
    .fs_file_id = 0,
    .mountpoint_id = 0,
    .buf_read_pos = 0,
    .buf_write_pos = 0,
    .file_size = get_file_size(file_header->size),
    .file_buffer = ((char*) file_header) + 512     //TODO: Replace 512 with sizeof(tar_header_t) 
  };
  strncpy(fd.filename, file_name, VFS_PATH_LENGTH);
  
  return fd;
}

size_t ustar_read(char* file_handle, char* buffer, size_t nbytes){
  memcpy((void*) buffer, file_handle, nbytes);
  return nbytes;
}

void tar_init(struct multiboot_info* mb_info){
  struct multiboot_tag_module* tag = (struct multiboot_tag_module*) mb_info->tags;
  for(; tag->type != MULTIBOOT_HEADER_TAG_END && tag->type != MULTIBOOT_TAG_TYPE_MODULE; tag = (struct multiboot_tag_module*) ALIGN_UP(((uintptr_t) tag) + tag->size, MULTIBOOT_INFO_ALIGN));

  if(tag->type != MULTIBOOT_TAG_TYPE_MODULE){
    PANIC("mb2_mmap", "Tag of Unknown type");
  }

  size_t file_size = PAGE_ALIGN(tag->mod_end - tag->mod_start);
  tar_header_t* tar_file = (tar_header_t*) vmm_alloc(get_mmio_ptr(file_size), file_size, VM_FLAG_READ_WRITE| VM_FLAG_USER, (void*) tag->mod_start); 
  memset((void*) file_headers, 0, HEADER_LIST_SIZE * sizeof(tar_header_t*));
  parse_tar_file(tar_file);
}
