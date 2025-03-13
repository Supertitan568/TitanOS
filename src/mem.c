#include "mem.h"
#include <stddef.h>
#include <stdint.h>

void* mem_cpy(void* dest, void* src, size_t num_bytes){
  // Getting something working quick
  // TODO: Rewrite with block copying in assembly

  uint8_t* current_dest = (uint8_t*) dest;
  uint8_t* current_src = (uint8_t*) src;

  for(int i = 0; i < num_bytes; i++){
    *current_dest = *current_src;
    current_dest++;
    current_src++;
  }

  return dest;
}

void* mem_set(void* s, int c, size_t n){
  uint8_t* current_s = (uint8_t*) s;
  for(int i = 0; i < n; i++){
    *current_s = c;
  }

  return s;
}

int mem_cmp(const void* s1, const void* s2, size_t n){
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
