#include "keyboard.h"
#include "vga.h"
#include "cpu_ports.h"
#include <stdint.h>

#define DATA_PORT 0x60
// Gotta come up with a better name. It has two purposes

#define SPECIAL_PORT 0x64

void keyboard_handler(){
  uint8_t status =inb(DATA_PORT);
  printstr("Incoming PS2 byte: ");
  printlong((uint64_t) status);
}
