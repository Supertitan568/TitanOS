#ifndef SCHEDULER_H
#include "vmm.h"

cpu_context_t* schedule(cpu_context_t* context);
void sched_init(vmm_t initial_vmm);

#endif // !SCHEDULER_H
