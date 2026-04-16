#ifndef RESOURCE_HANDLING_H
#define RESOURCE_HANDLING_H

#include <stddef.h>

typedef enum{
  FILE,
  MESSAGE_ENDPOINT,
  SHARED_MEM
}resource_type_t;

typedef struct resource_t resource_t;
typedef struct process_t process_t;

typedef struct resource_t{
  resource_type_t type;
  int internal_handle;

  size_t (*read)(resource_t* resource, void* buf, size_t len);
  size_t (*write)(resource_t* resource, void* buf, size_t len);
  void (*open)(resource_t* resource);
  void (*close)(resource_t* resource);

} resource_t;

int register_resource(process_t* process, resource_t* resource);
int register_file_resource(const char* filename);
#endif // !RESOURCE_HANDLING_H

