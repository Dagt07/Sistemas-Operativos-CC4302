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


void trash_function(nThread th){
  nth_delQueue(th->ptr, th); //revisar puntero
  th->ptr = NULL;
}


int nEnterWrite(nRWLock *rwl, int timeout) {
  START_CRITICAL

  nThread this_th = nSelf();
  this_th->ptr = &rwl->writers_queue;

  if(rwl->readers_count == 0 && !rwl->writing) {
    rwl->writing = 1;
  }
  else{
    if(timeout>0){ // Caso espera con timeout
      this_th = nSelf();
      nth_putBack(rwl->writers_queue, this_th); //REFAC esta linea arriba si funca
      this_th->ptr = &rwl->writers_queue; //enunciado
      suspend(WAIT_RWLOCK_TIMEOUT);
      nth_programTimer(timeout * 1000000LL, trash_function);
    }
    else{ // Caso espera indefinida (timeout <= 0)
      nth_putBack(rwl->writers_queue, this_th);
      suspend(WAIT_RWLOCK);
    }
    // Call the scheduler
    schedule();
  }

  int response = 1;
  //check if the nthread got successfully the RWLock or it was interrupted by a timeout
  if(this_th->ptr == NULL){ // timeout expired -> return 0
    response = 0;
  }
  
  END_CRITICAL
  return response;
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

  /* tengo dudas con esta parte 
  if(this_th->status == WAIT_RWLOCK || this_th->status == WAIT_RWLOCK_TIMEOUT){
    if(this_th->status == WAIT_RWLOCK_TIMEOUT){
        // cancelar timer si tiene timer
        nth_cancelThread(this_th);
    }
    setReady(this_th); //tengo dudas con esta parte
  }
  fin dudas */

  rwl->writing = 0;

  if(!nth_emptyQueue(rwl->readers_queue)){
    while(!nth_emptyQueue(rwl->readers_queue)){
      nThread reader_th = nth_getFront(rwl->readers_queue);
      if(reader_th->status == WAIT_RWLOCK_TIMEOUT){
        // cancelar timer si tiene timer
        nth_cancelThread(reader_th);
      }
      setReady(reader_th);
      rwl->readers_count++;
    }
    schedule();
  }
  
  else{
    if(!nth_emptyQueue(rwl->writers_queue)){
      nThread writer_th = nth_getFront(rwl->writers_queue);
      if(writer_th->status == WAIT_RWLOCK_TIMEOUT){
        // cancelar timer si tiene timer
        nth_cancelThread(writer_th);
      }
      setReady(writer_th);
      rwl->writing = 1;
      schedule();
    }
  }

  END_CRITICAL
}


void nExitRead(nRWLock *rwl) {
  START_CRITICAL

  /* tengo dudas con esta parte
  if(this_th->status == WAIT_RWLOCK || this_th->status == WAIT_RWLOCK_TIMEOUT){
    if(this_th->status == WAIT_RWLOCK_TIMEOUT){
        // cancelar timer si tiene timer
        nth_cancelThread(this_th);
    }
    setReady(this_th); 
  }
  fin dudas */

  rwl->readers_count--;
  if(rwl->readers_count == 0 && !nth_emptyQueue(rwl->writers_queue)){
    nThread writer_th = nth_getFront(rwl->writers_queue);
    if(writer_th->status == WAIT_RWLOCK_TIMEOUT){
        // cancelar timer si tiene timer
        nth_cancelThread(writer_th);
    }
    rwl->writing = 1;
    setReady(writer_th);
    schedule();
  }

  END_CRITICAL
}