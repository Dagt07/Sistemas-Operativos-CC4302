#define _XOPEN_SOURCE 500

#include "nthread-impl.h"

#include "rwlock.h"

struct rwlock {
  NthQueue *readers_queue, *writers_queue;
  int writing;
  int readers_count;
};

nRWLock *nMakeRWLock() {
  nRWLock *rwl = malloc(sizeof(nRWLock));
  rwl->readers_queue = nth_makeQueue();
  rwl->writers_queue = nth_makeQueue();
  rwl->writing = 0;
  rwl->readers_count = 0;
  return rwl;
}

void nDestroyRWLock(nRWLock *rwl) {
  nth_destroyQueue(rwl->readers_queue);
  nth_destroyQueue(rwl->writers_queue);
  nFree(rwl);
}


int nEnterWrite(nRWLock *rwl, int timeout) {
  START_CRITICAL

  if(rwl->readers_count == 0 && !rwl->writing) {
    rwl->writing = 1;
  }
  else{
    nThread this_th = nSelf();
    nth_putBack(rwl->writers_queue, this_th);
    suspend(WAIT_RWLOCK);
    schedule();
  }

  END_CRITICAL
  return 1;
}


int nEnterRead(nRWLock *rwl, int timeout) {
  START_CRITICAL

  if(!rwl->writing && nth_emptyQueue(rwl->writers_queue)) {
    rwl->readers_count++;
  }else{
    nThread this_th = nSelf();
    nth_putBack(rwl->readers_queue, this_th);
    suspend(WAIT_RWLOCK);
    schedule();
  }

  END_CRITICAL
  return 1;
}


void nExitWrite(nRWLock *rwl) {
  START_CRITICAL

  rwl->writing = 0;

  if(!nth_emptyQueue(rwl->readers_queue)){
    while(!nth_emptyQueue(rwl->readers_queue)){
      setReady(nth_getFront(rwl->readers_queue));
      rwl->readers_count++;
    }
    schedule();
  }
  
  else{
    if(!nth_emptyQueue(rwl->writers_queue)){
      setReady(nth_getFront(rwl->writers_queue));
      rwl->writing = 1;
      schedule();
    }
  }

  END_CRITICAL
}


void nExitRead(nRWLock *rwl) {
  START_CRITICAL

  rwl->readers_count--;
  if(rwl->readers_count == 0 && !nth_emptyQueue(rwl->writers_queue)){
    rwl->writing = 1;
    setReady(nth_getFront(rwl->writers_queue));
    schedule();
  }

  END_CRITICAL
}