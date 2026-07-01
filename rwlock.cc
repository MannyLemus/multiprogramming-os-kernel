#include<stdio.h>
#include <pthread.h>
#include <iostream>


#include "rwlock.h"

//Your solution to implement each of the following methods
//
RWLock::RWLock() : readers(0),writers(0),writers_waiting(0) { //initialize all counters to 0
    pthread_cond_init(&writers_proceed,nullptr); //creates condition variable
    pthread_cond_init(&readers_proceed,nullptr); //creates condition variable
    pthread_mutex_init(&mutex,nullptr); //creates mutex
}

RWLock::~RWLock() { //cleans allocated resources
    pthread_cond_destroy(&readers_proceed);
    pthread_cond_destroy(&writers_proceed);
    pthread_mutex_destroy(&mutex);
}

void RWLock::startRead() {
    pthread_mutex_lock(&mutex);
    while (writers_waiting>0 || writers>0) { //while loop to wait while there is an active write or writers waiting, give write preference
        pthread_cond_wait(&readers_proceed,&mutex); //then let reader proceed
    }
    readers++; //increment readers
    pthread_mutex_unlock(&mutex);
}

void RWLock::doneRead() {
    pthread_mutex_lock(&mutex);
    readers--;
    if (writers_waiting>0 && readers==0) { //if there are no more readers, wake one writer that is waiting
        pthread_cond_signal(&writers_proceed); //signal to write
    }
    pthread_mutex_unlock(&mutex);
}

void RWLock::startWrite() {
    pthread_mutex_lock(&mutex);
    writers_waiting++;
    while (writers>0||readers>0) { //wait until there are no active reader and no active writers
        pthread_cond_wait(&writers_proceed,&mutex); //the let writer proceed
    }
    writers_waiting--; //decrement from waiting writers
    writers = 1;
    pthread_mutex_unlock(&mutex);
}

void RWLock::doneWrite() {
    pthread_mutex_lock(&mutex);
    writers = 0;
    if (writers_waiting>0) { //this will wake up a waiting writer, writer has priority ensuring no starvatoin
        pthread_cond_signal(&writers_proceed); //let writer process
    } else {
        pthread_cond_broadcast(&readers_proceed); // otherwise wake all waitng readers using broadcast
    }
    pthread_mutex_unlock(&mutex);
}
//
