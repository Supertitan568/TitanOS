/* VGA driver for kernel use
 * This was written so quick so I could work on other things
 * TODO: Fix cursor positioning 
 */

#include "vga.h"
#include "cpu_ports.h"
#include "mem.h"
#include "vmm.h"
#include <stdint.h>
#include <stdarg.h>

// TODO: Get this at run time from the vmm
#define VGA_PHYS_START_ADDR 0xb8000 
#define VGA_PHYS_END_ADDR 0xb8fa0
#define VGA_COMMAND 0x3d4
#define VGA_DATA 0x3d5
#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_LENGTH VGA_PHYS_END_ADDR - VGA_PHYS_START_ADDR

struct{
  vga* vga_out;
  int x;
  int y;
} cursor;

void move_cursor(){
  uint32_t pos = cursor.y * VGA_WIDTH + (cursor.x + 1);
  // Setting cursor low port to vga register
  outb(VGA_COMMAND, 0x0f);
  outb(VGA_DATA, (uint8_t) (pos & 0xff));

  // Setting high vga register
  outb(VGA_COMMAND, 0x0e);
  outb(VGA_DATA, (uint8_t) ((pos >> 8)&0xff));
}

static void scroll_down(){
  //TODO: Fix undefined use of memcpy
  //      It will work for now but might 
  //      break if I change the implementation

  memcpy((void*)cursor.vga_out,(void*) (cursor.vga_out + VGA_WIDTH), VGA_LENGTH);
  //mem_set((void*) (vga_start + (VGA_WIDTH * 24)), 0, VGA_WIDTH);
}

void printc(char c){
  if (c == '\n'){
    cursor.x = 0;
    cursor.y++;
  }
  else{
    vga* curr_char = (vga*) ((uintptr_t)cursor.vga_out + (((cursor.y * VGA_WIDTH) + cursor.x) * 2));
    *(curr_char) = 0x0f00 + c; 
    cursor.x += 1;
    cursor.y += (int) (cursor.x / ((int) VGA_WIDTH)); 
    cursor.x %= VGA_WIDTH;
  }
  
  if(cursor.y >= VGA_HEIGHT){
    cursor.x = 0;
    cursor.y = VGA_HEIGHT - 1;
    scroll_down();
  } 
}

void console_init(){
  // Resets console
  cursor.vga_out = (vga*) vmm_alloc((uintptr_t) NULL, VGA_LENGTH, VM_FLAG_MMIO, (void*) VGA_PHYS_START_ADDR);
   
  cursor.x = 0;
  cursor.y = 0;
  move_cursor();
}

void clear_console(){
  for(int i=0; i < 2000; i++){
    printc(' ');   
  }
  console_init();
}



void printstr(const char* str){
  char* current_char = (char*) str;
  while(*current_char != '\0'){
    printc(*current_char);
    current_char++;
  }

  move_cursor();
}

void printlong(uint64_t num){
  // Maybe not the best implementation lol
  uint8_t lsb;
  char num_str[17];

  for(int i = 15; i >= 0; i--){
    lsb = num & 0x0f;
    if(lsb < 0xa){
      lsb += 0x30; 
    } 
    else{
      lsb += 0x57; 
    }
    
    num >>= 4;
    num_str[i] = (char) lsb;
  }
  num_str[16] = '\0';
  printstr("0x");
  printstr(num_str);
}

static void printint(int d){ 
  char num_str[10];
  uint8_t digit;
  for(int i = sizeof(num_str); i > 0; i--){
    digit = d % 10;
    num_str[i - 1] += 48 + digit;
    d -= digit;
    if(d == 0)
      break; 
  }

  printstr(num_str);
}

void printf(const char* format, ...){
  va_list args;
  va_start(args, format);
  char* current_char = (char*) format;
  while(*current_char != '\0'){
    if(*current_char == '%'){
      switch(*(current_char + 1)){
        case 'd':
          printint(va_arg(args, int));
          current_char += 2;
          continue;
        case 's':
          printstr(va_arg(args, char*));
          current_char += 2;

        case 'l':
          if(*(current_char + 2) == 'u'){
            printlong(va_arg(args, uint64_t));
            current_char += 3;
            continue;
          }
          break;
      }
    }
    printc(*current_char);
    current_char++;
  }
   
  move_cursor();
}
