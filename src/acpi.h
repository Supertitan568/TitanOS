#ifndef ACPI_H
#include <stdint.h>
#include <stdbool.h>

struct __attribute__((packed)) rsdp_descriptor_t {
  char signature[8];
  uint8_t checksum;
  char oemid[6];
  uint8_t revision;
  uint32_t rsdt_addr;
};

struct acpi_sdt_header_t {
  char signature[4];
  uint32_t length;
  uint8_t revision;
  uint8_t checksum;
  char oemid[6];
  char oemid_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
};

struct rsdp_t {
  struct acpi_sdt_header_t sdt_header;
  uint32_t sdt_addr[];
};

struct rsdp_descriptor_t* get_rsdp();

bool validate_rsdp(struct rsdp_descriptor_t* rsdp);
struct rsdp_t* get_apic_table(struct rsdp_descriptor_t* rsdp_desc);

#endif 
