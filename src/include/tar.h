#ifndef TAR_H
#define TAR_H
#include <multiboot2.h>
#include <vfs.h>

file_descriptor_t ustar_open(const char* file_name, int flags);

size_t ustar_read(char* file_handle, char* buffer, size_t nbytes);
int ustar_close(int ustar_fd);

void tar_init(struct multiboot_info* mb_info);

#endif // !TAR_H
