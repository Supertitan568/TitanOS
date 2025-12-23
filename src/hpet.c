#include "acpi.h"
#include "vga.h"
#include <stdint.h>

void hpet_init(){
  struct rsdp_t* hpet_tbl = get_acpi_table(get_rsdp(), "HPET");
  printstr("HPET Location: ");
  printlong((uint64_t) hpet_tbl);
  printc('\n');
}
