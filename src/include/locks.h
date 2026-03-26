#ifndef LOCKS_H
#define LOCKS_H
#include <stdbool.h>

typedef struct {
  bool locked;
}spinlock_t;

void acquire(spinlock_t* lock);
void release(spinlock_t* lock);


#endif // !LOCKS_H
