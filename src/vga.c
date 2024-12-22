/* VGA driver for kernel use
 * This was written so quick so I could work on other things
 * TODO: Fix cursor positioning 
 */

#include "vga.h"
#include "cpu_ports.h"
#include <stdint.h>
#define VGA_START_ADDR 0xb8000
#define VGA_END_ADDR 0xb8fa0
#define VGA_COMMAND 0x3d4
#define VGA_DATA 0x3d5
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

struct{
  vga* vga_out;
  int x;
  int y;
} cursor;

static void move_cursor(int x, int y){
  int pos = y * VGA_WIDTH + x;
  // Setting cursor low port to vga register
  outb(VGA_COMMAND, 0x0f);
  outb(VGA_DATA, (uint8_t) (pos & 0xff));

  // Setting high vga register
  outb(VGA_COMMAND, 0x0e);
  outb(VGA_DATA, (uint8_t) ((pos >> 8)&0xff));
}

static void printc(char c){
  if (c == '\n'){
    cursor.x = 0;
    cursor.y++;
    return;
  }
  else{
    cursor.x = (cursor.x + 1) % VGA_WIDTH;
    cursor.y += (int) (cursor.x / VGA_WIDTH); 
  }
  
  if(cursor.y >= VGA_HEIGHT){
    cursor.y = VGA_HEIGHT - 1;   
  }
  
  cursor.vga_out = (vga*) (VGA_START_ADDR + (((cursor.y * VGA_WIDTH) + cursor.x) * 2));
  *(cursor.vga_out) = 0x0f00 + c;
}

void console_init(){
  // Resets console
  cursor.vga_out = (vga*) VGA_START_ADDR;
  cursor.x = 0;
  cursor.y = 0;
  move_cursor(cursor.x, cursor.y);
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

  move_cursor(cursor.x + 1, cursor.y);
}



