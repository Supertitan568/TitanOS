#ifndef VGA_H
#include <stdint.h>

void console_init();
void printstr(const char* str);
void clear_console();
void printlong(uint64_t num);
void printc(char c);
void printf(const char* format, ...);
void move_cursor();

typedef volatile unsigned short vga;

#endif // !VGA_H    
