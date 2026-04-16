#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "vmm.h"
#include <resource_handling.h>

cpu_context_t* schedule(cpu_context_t* context);
void sched_init(vmm_t initial_vmm);

typedef struct process_t process_t;


typedef enum {
  READY,
  RUNNING,
  DEAD
} status_t;


typedef struct thread_t{
  size_t tid;
  process_t* parent;
  cpu_context_t context;
  void* stack;
  status_t status;
#define NAME_MAX 10
  char name[NAME_MAX];
  struct thread_t* next;
} thread_t;

struct process_t{
  size_t pid;
  thread_t* threads; 
  vmm_t vmm;
  char name[NAME_MAX];
#define MAX_RESOURCES 5
  resource_t* resource_table[MAX_RESOURCES];
  struct process_t* next;
};

extern process_t* current_process;

#endif // !SCHEDULER_H
