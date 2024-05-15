#define _XOPEN_SOURCE 500

#include "nthread-impl.h"

#include "rwlock.h"

struct rwlock {
  NthQueue *readers, *writers;
  int writing;
  int readers_count;
};

nRWLock *nMakeRWLock() {
  nRWLock *rwl = malloc(sizeof(nRWLock));
  rwl->readers = nth_makeQueue();
  rwl->writers = nth_makeQueue();
}

void nDestroyRWLock(nRWLock *rwl) {
  nth_destroyQueue(rwl->readers);
  nth_destroyQueue(rwl->writers);
}

int nEnterRead(nRWLock *rwl, int timeout) {
  START_CRITICAL

  if

  END_CRITICAL
  return 1;
}

int nEnterWrite(nRWLock *rwl, int timeout) {
  START_CRITICAL

  END_CRITICAL
  return 1;
}

void nExitRead(nRWLock *rwl) {
  START_CRITICAL

  END_CRITICAL
}

void nExitWrite(nRWLock *rwl) {
  START_CRITICAL

  END_CRITICAL
}
