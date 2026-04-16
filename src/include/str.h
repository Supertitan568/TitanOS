#ifndef STR_H
#define STR_H
#include <stddef.h>

size_t strnlen(const char s[], size_t maxlen);
char* strncpy(char* dst, const char* src, size_t dsize);
int strncmp(const char *s1, const char *s2, size_t n);

#endif // !STR_H


