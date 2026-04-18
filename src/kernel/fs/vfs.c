#include <vfs.h>
#include <kernel_heap_manager.h>
#include <str.h>
#include <libtitan.h>
#include <tar.h>
#include <resource_handling.h>
#include <scheduler.h>

#define MAX_MOUNTPOINTS 12
mountpoint_t* mountpoints[MAX_MOUNTPOINTS];
#define MAX_OPENED_FILES 10
file_descriptor_t vfs_opened_files[MAX_OPENED_FILES];

static mountpoint_t* create_mountpoint(char* device, char* target, char* fs_type){
  mountpoint_t* new_mountpoint = (mountpoint_t*) kmalloc(sizeof(mountpoint_t));
  
  strncpy(new_mountpoint->device, device, VFS_PATH_LENGTH);
  strncpy(new_mountpoint->type, fs_type, 32);
  strncpy(new_mountpoint->mountpoint, target, VFS_PATH_LENGTH);
  
  if(strncmp(fs_type, "tar", VFS_PATH_LENGTH) == 0){
    new_mountpoint->open = ustar_open;
    new_mountpoint->read = ustar_read;
  }
  else{
    kfree(new_mountpoint);
    return NULL;
  }

  return new_mountpoint;
}


static bool add_mountpoint(mountpoint_t* mountpoint){
  for(int i=0; i<MAX_MOUNTPOINTS; i++){
    if(!mountpoints[i]){
      mountpoints[i] = mountpoint;
      return true;
    }
  }

  return false;
}


static mountpoint_t* get_mountpoint(const char* path){
  int i=0;
  for(; i<MAX_MOUNTPOINTS; i++){
    if(strncmp(mountpoints[i]->mountpoint, path, VFS_PATH_LENGTH) == 0){
      return mountpoints[i];
    }
  }
  return NULL;
}


static void remove_mountpoint(mountpoint_t* mountpoint){
  for(int i=0; i<MAX_MOUNTPOINTS; i++){
    if(mountpoints[i] == mountpoint){
      mountpoints[i] = 0;
      break;
    }
  }
  kfree(mountpoint);
}


bool vfs_mount(char* device, char* target, char* fs_type){
  if(!add_mountpoint(create_mountpoint(device, target, fs_type))){
    PANIC("vfs", "failed to mount fs");
  }
  return true;
}


// Replace with bool
int vfs_umount(char* device, char* target){
  int i=0;
  for(; i<MAX_MOUNTPOINTS; i++){
    if(strncmp(mountpoints[i]->device, device, VFS_PATH_LENGTH) == 0){
      mountpoints[i] = 0;
      break;
    }
  }
  kfree(mountpoints[i]);

  return 1;
}


// Returns a handle
int vfs_open(const char* filename, size_t flags){
  // TODO: Properly parse the filepath
  mountpoint_t* mountpoint = get_mountpoint(filename);

  // Setting up the file according to the filesystem specified in the mountpoint
  file_descriptor_t fd;
  if(strncmp("tar", mountpoint->type, 3) == 0){
    fd = ustar_open(filename + 1, 0);
  }
  else{
    ERROR("vfs", "could not determine FS type");
    goto error; 
  }
  
  for(int i=0; i<MAX_OPENED_FILES; i++){
    if(*vfs_opened_files[i].filename == '\0'){
      vfs_opened_files[i] = fd;
      return i;
    } 
  }
  ERROR("vfs", "reached max amount of files");

  error:
  return -1;
}


bool vfs_close(int file_handle){
  return true;
}


size_t vfs_read(int file_handle, void* buf, size_t nbytes){
  size_t bytes_read = 0;
  if(strncmp("tar", mountpoints[vfs_opened_files[file_handle].mountpoint_id]->type, 3) == 0){
    bytes_read = ustar_read(vfs_opened_files[file_handle].file_buffer + vfs_opened_files[file_handle].buf_read_pos, buf, nbytes);
  }
  else{
    ERROR("vfs", "could not determine FS type");
    goto error;
  }

  vfs_opened_files[file_handle].buf_read_pos += bytes_read;
  
  success:
  return bytes_read;
  error: 
  return -1;
}

size_t vfs_seek(int file_handle, size_t pos){
  vfs_opened_files[file_handle].buf_read_pos = pos;
  return pos;
}

bool vfs_init(){
  mountpoint_t* init_mountpoint = create_mountpoint("initramfs", "/", "tar");
  if(!add_mountpoint(init_mountpoint)){
    PANIC("vfs", "could not mount initramfs");
  }

  return true;
}
