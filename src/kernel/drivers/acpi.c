#include "acpi.h"
#include <stdbool.h>
#include <stdint.h>
#include "mem.h"
#include "vga.h"
#include "vmm.h"

#define EXTENDED_BIOS_START_ADDR 0x000e0000
#define EXTENDED_BIOS_END_ADDR 0x000fffff
#define EXTENDED_BIOS_LENGTH EXTENDED_BIOS_END_ADDR - EXTENDED_BIOS_START_ADDR 
#define RSDP_SIG_SIZE 8

// TODO: Add support for XSDT

static bool check_apic(){   
  return true;
}

static inline void print_signature(struct acpi_sdt_header_t* acpi_header){
  for(int i=0; i<4; i++){
    printc(acpi_header->signature[i]);
  }
}

static void print_rsdt_info(struct rsdp_t* rsdt_ptr){
  printstr("RSDT Addr: ");
  printlong((uint64_t) rsdt_ptr);
  printc('\n');

  printstr("RSDT Length: ");
  printlong(rsdt_ptr->sdt_header.length);
  printc('\n'); 
  
  printstr("RSDT Signature: ");
  print_signature(&rsdt_ptr->sdt_header); 
  printc('\n');
}


struct rsdp_descriptor_t* get_rsdp(){
  
  static char* current_sig_ptr = NULL;
  if(current_sig_ptr != NULL)
    return current_sig_ptr;

  

  current_sig_ptr = (char*) vmm_alloc((uintptr_t) NULL, EXTENDED_BIOS_LENGTH, VM_FLAG_MMIO,(void*) EXTENDED_BIOS_START_ADDR); 
  if(check_apic()){
    char* end_ptr = (char*) EXTENDED_BIOS_END_ADDR;
    for(; current_sig_ptr < end_ptr; current_sig_ptr = current_sig_ptr + 16){
      if(memcmp(current_sig_ptr, "RSD PTR ", RSDP_SIG_SIZE)){
        return (struct rsdp_descriptor_t*) current_sig_ptr;
      }
    } 
  }
 return NULL;
}


bool validate_rsdp(struct rsdp_descriptor_t* rsdp){
  uint8_t* byte_ptr = (uint8_t*) rsdp;
  uint32_t checksum = 0;

  for(int i = 0; i < sizeof(struct rsdp_descriptor_t); i++){
    checksum += byte_ptr[i];
  }

  return (checksum & 0xff) == 0;
}


struct rsdp_t* get_acpi_table(struct rsdp_descriptor_t* rsdp_desc, const char* sig){
  static struct rsdp_t* rsdt_ptr = NULL;
  if(rsdt_ptr == NULL){
    rsdt_ptr = vmm_alloc((uintptr_t) NULL, 0x2000, VM_FLAG_MMIO, rsdp_desc);
  }


  // Skill issue with C right here
  uint32_t* sdt_entry_array = (uint32_t*) &rsdt_ptr->sdt_addr; 
  struct acpi_sdt_header_t* sdt_entry;

  // Praying that this doesn't fail
  uint32_t rsdt_length = (rsdt_ptr->sdt_header.length - sizeof(struct acpi_sdt_header_t)) / 4; 
  for(uint8_t i = 0; i < rsdt_length; i++){
    sdt_entry = (struct acpi_sdt_header_t*) sdt_entry_array[i];
    if(memcmp(sdt_entry->signature, sig, 4)){
      printstr("Found ACPI table at ");
      printlong((uint64_t) sdt_entry);
      printc('\n');

      return (struct rsdp_t*) sdt_entry; 
    }  
  }
  
 return 0;
}
