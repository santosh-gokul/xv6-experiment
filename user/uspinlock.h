#ifndef __UTHREAD_H__
#define __UTHREAD_H__

#include <stdbool.h>
#include <stddef.h>


struct uspinlock {
  int locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  int tid;;   // The tid holding the lock.
};

#endif
