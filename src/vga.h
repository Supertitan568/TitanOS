#ifndef VGA_H
void console_init();
void printstr(const char* str);
void clear_console();
typedef volatile unsigned short vga;

#endif // !VGA_H    
