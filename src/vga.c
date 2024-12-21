#include "vga.h"
#define VGA_START_ADDR 0xb8000
const char* string = "zsdf";

// struct{
//   vga* vga_out;
//   int x;
//   int y;
// }cursor;
//
// void console_init(){
//   cursor.vga_out = (vga*) VGA_START_ADDR;
//   cursor.x = 0;
//   cursor.y = 0;
// }

void printc(char c){
  vga* vga_out = (vga*) VGA_START_ADDR;
  *(vga_out) = 0x0f00 + *(string); 
}

