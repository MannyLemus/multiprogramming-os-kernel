#ifndef __RWLOCK_H__
#define __RWLOCK_H__
#include<semaphore.h>

class RWLock{
private:
    //Your solution to add more private fields
    int readers, writers, writers_waiting;
    //readers: amount of active readers, could be mulitple
    //writers: 0 or 1 active writers, only up to one writer
    //waiting_writers: amount of writers waiting
    pthread_mutex_t mutex; //mutual exclusion lock for critical section access
    pthread_cond_t readers_proceed; //reader may proced condition var
    pthread_cond_t writers_proceed; //writer may proceed condition var

public:
    RWLock();
    ~RWLock();
    //Reader
    void startRead();
    void doneRead();
    // Writer
    void startWrite();
    void  doneWrite();
};

#endif
