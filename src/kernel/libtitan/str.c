#include <mem.h>
#include <str.h>

size_t strnlen(const char s[], size_t maxlen){
  for(int i=0; i<maxlen; i++){
    if(s[i] == '\0'){
      return i;
    } 
  }
  return maxlen;
}


char* strncpy(char* dst, const char* src, size_t dsize){
  size_t src_len = strnlen(src, dsize);
  memcpy((void*) dst, (void*) src, src_len);
  memset((void*) (dst + src_len), 0, dsize - src_len);
  return dst;
}


int strncmp(const char *s1, const char *s2, size_t n){
  int i = 1;
  for(; i<n; i++){
    if(!s1[i] || !s2[i]){
      return 0; 
    }
    else if (s1[i] != s2[i]) {
      break;      
    }
  }

  return s1[i] - s2[i];
}
