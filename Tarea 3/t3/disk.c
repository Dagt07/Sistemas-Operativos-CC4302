#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "disk.h"
#include "pss.h"

/*****************************************************
 * Add types, global variables and functions u need here */

typedef struct {
  int ready;
  int track;
  pthread_cond_t condition;
} Request;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
PriQueue *greaterThanQueue, *lowerThanQueue;
int busy = 0;
int actual_track = 0;

/*****************************************************/

void iniDisk(void) {
  // Initialize queues
  greaterThanQueue = makePriQueue();
  lowerThanQueue = makePriQueue();
}


void cleanDisk(void) {
  // Destroy queues
  destroyPriQueue(greaterThanQueue);
  destroyPriQueue(lowerThanQueue);
}


void requestDisk(int track) {
  /* Request to use the track, the track gives the priority if needed*/
  pthread_mutex_lock(&mutex);
  if(!busy){
    busy = 1;
    actual_track = track;
  }
  else{
    Request req = {0,track,PTHREAD_COND_INITIALIZER};
    // we check where to put the track
    if(track<actual_track){
      priPut(lowerThanQueue, &req, track);
    }
    else{
      priPut(greaterThanQueue, &req, track);
    }
    while(!req.ready){
      pthread_cond_wait(&req.condition, &mutex);
      actual_track = req.track;
    }
  }
  pthread_mutex_unlock(&mutex);
}


void releaseDisk(){
  /* Realease the disk according with the C-SCAN strategy */
  pthread_mutex_lock(&mutex);

  if(emptyPriQueue(greaterThanQueue) && emptyPriQueue(lowerThanQueue)){
    busy = 0;
    pthread_mutex_unlock(&mutex);
    return;
  }
  else{
    if(emptyPriQueue(greaterThanQueue)){
      //we swap the elements from lowerThanQueue to greaterThanQueue
      while(!emptyPriQueue(lowerThanQueue)){
        Request *req = priGet(lowerThanQueue);
        priPut(greaterThanQueue, req, req->track);
      }
    }
    if(!emptyPriQueue(greaterThanQueue)){
      Request *req = priGet(greaterThanQueue);
      req->ready = 1;
      pthread_cond_signal(&req->condition);
    }
  }

  pthread_mutex_unlock(&mutex);
}
