#ifndef APIC_H
#include <stdint.h>

#define IOAPICID 0x0
#define IOAPICVER 0x1
#define IOAPICARB 0x2
#define IOREDTBL(i) (2 * i + 0x10)

void disable_8259();
void local_apic_enable(void* local_apic_regs);
inline uint64_t get_apic_id_reg(void* local_apic_regs){
  return *((uint64_t*) local_apic_regs);
}
void remap_ioredtbl(void* io_apic_base);
void send_local_apic_eoi(void* local_apic_regs);
void apic_init();

#endif // APIC_H
