#ifndef ACPI_H
#include <multiboot2.h>

typedef struct __attribute__((packed)) rsdp_t {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_addr;
}rsdp_t;

typedef struct acpi_sdt_header_t {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oemid_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
}acpi_sdt_header_t;

typedef struct rsdt_t {
  acpi_sdt_header_t sdt_header;
  uint32_t sdt_addr[];
} rsdt_t;


acpi_sdt_header_t* get_acpi_table(const char* sig);
bool acpi_init(struct multiboot_info* mb2_info);

#endif 
