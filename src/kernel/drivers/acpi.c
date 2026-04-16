#include <acpi.h>
#include <vga.h>
#include <vmm.h>
#include <kernel_heap_manager.h>
#include <mem.h>
#include <libtitan.h>

#define EXTENDED_BIOS_START_ADDR 0x000e0000
#define EXTENDED_BIOS_END_ADDR 0x000fffff
#define EXTENDED_BIOS_LENGTH EXTENDED_BIOS_END_ADDR - EXTENDED_BIOS_START_ADDR 
#define RSDP_SIG_SIZE 8

rsdt_t* rsdt = NULL;

// This is necessary because the rsdt only stores physical memory pointers
struct{
  size_t length;
  acpi_sdt_header_t** tables;
}acpi_tables;

static inline void print_signature(acpi_sdt_header_t* acpi_header){
  for(int i=0; i<4; i++){
    printc(acpi_header->signature[i]);
  }
}


static inline void print_oem_signature(acpi_sdt_header_t* acpi_header){
  for(int i=0; i<6; i++){
    printc(acpi_header->oemid[i]);
  }
}


static void print_rsdt_info(rsdt_t* rsdt_ptr){
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


static rsdp_t* get_rsdp(struct multiboot_info* mb2_info){
  static char* current_sig_ptr = NULL;
  if(current_sig_ptr != NULL)
    return (rsdp_t*) current_sig_ptr;

  struct multiboot_tag_new_acpi* xsdp_tag = (struct multiboot_tag_new_acpi*) get_mb2_tag(mb2_info, MULTIBOOT_TAG_TYPE_ACPI_NEW);
  
  if(xsdp_tag != NULL){
    current_sig_ptr = (char*) xsdp_tag->rsdp;
    return (rsdp_t*) xsdp_tag->rsdp;
  }

  current_sig_ptr = (char*) vmm_alloc((uintptr_t) NULL, EXTENDED_BIOS_LENGTH, VM_FLAG_MMIO,(void*) EXTENDED_BIOS_START_ADDR); 
  char* end_ptr = (char*) EXTENDED_BIOS_END_ADDR;
  for(; current_sig_ptr < end_ptr; current_sig_ptr += 16){
    if(memcmp(current_sig_ptr, "RSD PTR ", RSDP_SIG_SIZE)){
      return (rsdp_t*) current_sig_ptr;
    }
  } 
 return NULL;
}


static acpi_sdt_header_t* map_table(acpi_sdt_header_t* table){
  
  size_t offset = ((size_t)table) - (((size_t)table) & 0xfffffffffffff000);
  void* new_table_page = (void*) alloc_mmio(table, PAGE_SIZE * 2, VM_FLAG_NONE);
  acpi_sdt_header_t* new_table = (acpi_sdt_header_t*)((uintptr_t) new_table_page + offset); 

  if(new_table->length > (PAGE_SIZE * 2)){
    alloc_mmio(table + PAGE_SIZE * 2, PAGE_ALIGN(table->length - PAGE_SIZE * 2), VM_FLAG_NONE);
  }
  return new_table; 
}


static bool validate_checksum(uint8_t* p, size_t length){
  uint32_t checksum = 0;

  for(int i = 0; i < length; i++){
    checksum += p[i];
  }

  return (checksum & 0xff) == 0;
}


acpi_sdt_header_t* get_acpi_table(const char* sig){
  for(int i = 0; i < acpi_tables.length; i++){
    if(memcmp(acpi_tables.tables[i]->signature, sig, 4)){
      return (acpi_sdt_header_t*) acpi_tables.tables[i]; 
    }  
  }
  
 return NULL;
}


static void dump_acpi_info(){
  if(rsdt != NULL && acpi_tables.tables != NULL){
    printf("acpi: ACPI Version %d\n", rsdt->sdt_header.revision);
    printf("acpi: ACPI OEM ");
    print_oem_signature(&rsdt->sdt_header);
    printc('\n');
    for(int i=0; i<acpi_tables.length; i++){
      printf("acpi: found the ");
      print_signature(acpi_tables.tables[i]);
      printc('\n');
    }
    
  }
}

static void parse_rsdt(){
  if(rsdt != NULL){
    acpi_tables.length  = (rsdt->sdt_header.length - sizeof(acpi_sdt_header_t)) / 4;
    acpi_tables.tables = (acpi_sdt_header_t**) kmalloc(sizeof(acpi_sdt_header_t*) * acpi_tables.length);
    
    for(int i=0; i<acpi_tables.length; i++){
      acpi_tables.tables[i] = map_table((void*)rsdt->sdt_addr[i]);
      if(!validate_checksum((uint8_t*) acpi_tables.tables[i], acpi_tables.tables[i]->length)){
        ERROR("acpi", "table checksum failed");
      }
    }
  }
}

bool acpi_init(struct multiboot_info* mb2_info){
  // TODO: Support the xsdt
  rsdp_t* rsdp = get_rsdp(mb2_info);
  if(rsdp == NULL || !validate_checksum((uint8_t*) rsdp, sizeof(rsdp_t))){
    PANIC("acpi", "rsdp not valid");
  }
  rsdt = (rsdt_t*) get_mmio_ptr(2 * PAGE_SIZE);
  rsdt = (rsdt_t*) vmm_alloc((uintptr_t)rsdt, 2 * PAGE_SIZE, VM_FLAG_READ_WRITE, (void*) (rsdp->rsdt_addr));
  rsdt = (rsdt_t*) (((uintptr_t) rsdt) + ((uintptr_t)rsdp->rsdt_addr) - (((uintptr_t)rsdp->rsdt_addr) & 0xfffffffffffff000));
  parse_rsdt();
  dump_acpi_info();

  return true;
}
