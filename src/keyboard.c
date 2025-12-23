#include "keyboard.h"
#include "vga.h"
#include "cpu_ports.h"
#include <stdint.h>
#include <stdbool.h>

#define DATA_PORT 0x60
#define COMMAND_PORT 0x64
#define STATUS_PORT 0x64
#define KEYBOARD_BUFFER_SIZE 32

static inline bool is_full(){
  return (inb(STATUS_PORT) >> 1) & 1;
}

// Circular buffer for now
// Imma implement a proper memcpy before making a better one
uint8_t keyboard_buffer[KEYBOARD_BUFFER_SIZE];
uint8_t keyboard_pos = 0;

// We are using scanset 1. Rip if you use 2 or 3
uint8_t keyboard_layout[128] = {
  0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 
  '-', '=', 0, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
  '[', ']', 0, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'',
  '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0
};

bool is_make = true;

void keyboard_handler(){
  uint8_t status =inb(DATA_PORT) & 0x7f;
  if(is_make){
    status = keyboard_layout[status];
    if(status){
      printc(status);
      move_cursor();
      keyboard_buffer[keyboard_pos] = status;
      keyboard_pos = (keyboard_pos + 1) % KEYBOARD_BUFFER_SIZE; 
    }
    is_make = false;
  }
  else{
    is_make = true;
  }
}

void check_scanset(){
  // Attempting to send the get scanset command
  // I failed pretty hard at this. I will come back when I have more motivation
  do{
    if(is_full()){
      outb(DATA_PORT, 0xf0);
      
      while(is_full())
        outb(DATA_PORT, 0x00);

      break;
    }
  }
  while(inb(DATA_PORT) == 0xfe);

  printstr("Response from keyboard: ");
  printlong((uint64_t) inb(DATA_PORT));
  printc('\n');
  printlong((uint64_t) inb(DATA_PORT));
  printc('\n');

}
