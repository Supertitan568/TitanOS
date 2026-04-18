#ifndef VFS_H
#define VFS_H
#include <resource_handling.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define VFS_TYPE_LEGNTH 32
#define VFS_PATH_LENGTH 64

typedef struct {
  uint64_t fs_file_id;
  int mountpoint_id;
  char filename[VFS_PATH_LENGTH];
  int buf_read_pos;
  int buf_write_pos;
  int file_size;
  char *file_buffer;
} file_descriptor_t;

typedef struct {
  char type[VFS_TYPE_LEGNTH];
  char device[VFS_PATH_LENGTH];
  char mountpoint[VFS_PATH_LENGTH];
   
  file_descriptor_t (*open)(const char* file_name, int flags);
  size_t (*read)(char* file_handle, char* buffer, size_t nbytes);
} mountpoint_t;


int vfs_open(const char* filename, size_t flags);
bool vfs_close(int file_handle);

size_t vfs_seek(int file_handle, size_t pos);
size_t vfs_read(int file_handle, void* buf, size_t nbytes);
bool vfs_init();

#endif
