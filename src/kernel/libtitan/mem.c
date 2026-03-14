#include "mem.h"
#include <stddef.h>
#include <stdint.h>

void* memcpy(void* dest, void* src, size_t n){
  // Getting something working quick
  // TODO: Rewrite with block copying in assembly
  void* curr_dest = dest;
  asm volatile ("cld" ::: "cc");
  asm volatile ("rep movsb" : "+D"(curr_dest) , "+S"(src) , "+c"(n) ::"memory");
  return dest;
}

void* memset(void* s, int c, size_t n){
  uint8_t* current_s = (uint8_t*) s;
  for(int i = 0; i < n; i++){
    current_s[i] = c;
  }

  return s;
}

int memcmp(const void* s1, const void* s2, size_t n){
  uint8_t* current_s1 = (uint8_t*) s1;
  uint8_t* current_s2 = (uint8_t*) s2;

  for(int i = 0; i < n; i++){
    if(*current_s1 != *current_s2){
      return 0; 
    }
    current_s1++;
    current_s2++;
  }
  return 1;
}
