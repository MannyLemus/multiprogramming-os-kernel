# Multiprogramming OS Kernel — Synchronization

## Overview

This project implements synchronization support for a multithreaded hash table and a small operating-system-style thread environment. The main goal was to make shared hash map operations safe under concurrent access while supporting multiple synchronization strategies: semaphores, locks and reader-writer locks.

The implementation includes fine-grained synchronization at the hash-bucket level, a custom reader-writer lock and Nachos-style `Lock` and `Condition` primitives. The synchronized hash map supports concurrent `get`, `put`, `remove` and `increment` operations while preventing race conditions between readers and writers.

## Features

- Fine-grained hash table synchronization using one synchronization object per hash bucket.
- Compile-time synchronization selection through preprocessor flags.
- Semaphore-based mutual exclusion.
- Lock-based mutual exclusion.
- Reader-writer lock synchronization allowing multiple concurrent readers.
- Writer-priority reader-writer locking to prevent writer starvation.
- Nachos-style `Lock` implementation using interrupt disabling and a wait queue.
- Nachos-style `Condition` implementation with `Wait`, `Signal` and `Broadcast`.
- Pthreads-based reader-writer lock implementation using mutexes and condition variables.
- Test targets for both simple and comprehensive multithreaded hash map validation.

## Technologies Used

- C++
- Pthreads
- Nachos-style thread synchronization
- Semaphores
- Mutex locks
- Condition variables
- Reader-writer locks
- Makefile-based testing

## Key Design Ideas

### Fine-Grained Hash Bucket Synchronization

The hash table uses separate synchronization objects for each hash slot instead of one global lock. This allows unrelated keys in different buckets to be accessed in parallel.

The synchronization behavior is selected at compile time with macros:

```cpp
START_READ()
END_READ()
START_WRITE()
END_WRITE()
```

Depending on the compile-time flag, these macros expand into semaphore operations, lock operations, reader-writer lock operations, or no synchronization.

For example, when reader-writer locking is enabled, a read operation calls:

```cpp
rwlck[hash]->startRead();
rwlck[hash]->doneRead();
```

and a write operation calls:

```cpp
rwlck[hash]->startWrite();
rwlck[hash]->doneWrite();
```

This kept the main hash table logic clean while making it easy to switch between synchronization strategies.

## Reader-Writer Lock

The `RWLock` class supports multiple simultaneous readers but only one active writer. The Pthreads version keeps track of:

- `readers`: number of active readers
- `writers`: whether a writer is active
- `writers_waiting`: number of waiting writers

It uses one mutex to protect the shared state and two condition variables:

- `readers_proceed`
- `writers_proceed`

The lock gives priority to waiting writers. Once a writer arrives, new readers wait instead of repeatedly entering and starving the writer.

### Read Flow

```cpp
startRead()
```

- Acquires the mutex.
- Waits while a writer is active or a writer is waiting.
- Increments the active reader count.
- Releases the mutex.

```cpp
doneRead()
```

- Acquires the mutex.
- Decrements the active reader count.
- If this was the last reader and a writer is waiting, signals one writer.
- Releases the mutex.

### Write Flow

```cpp
startWrite()
```

- Acquires the mutex.
- Increments the waiting writer count.
- Waits until no readers or writers are active.
- Marks one writer as active.
- Releases the mutex.

```cpp
doneWrite()
```

- Acquires the mutex.
- Clears the active writer flag.
- Signals another waiting writer if one exists.
- Otherwise broadcasts to all waiting readers.
- Releases the mutex.

## Nachos-Style Lock and Condition Variables

The Nachos version implements synchronization primitives using interrupt disabling for atomicity.

### Lock

The `Lock` class tracks:

- lock availability
- the current owning thread
- a queue of waiting threads

`Acquire()` disables interrupts, waits if the lock is unavailable and records the current thread as the owner. `Release()` verifies ownership, releases the lock and wakes a waiting thread if one exists.

### Condition

The `Condition` class provides:

- `Wait(Lock*)`
- `Signal(Lock*)`
- `Broadcast(Lock*)`

`Wait()` releases the associated lock, places the current thread on the condition queue, puts the thread to sleep and reacquires the lock after being signaled. `Signal()` wakes one waiting thread, while `Broadcast()` wakes all waiting threads.

## Hash Map Synchronization

The hash map protects public operations with read or write synchronization:

```cpp
int HashMap::get(int key)
```

Uses read synchronization because it only searches the bucket.

```cpp
void HashMap::put(int key, int value)
void HashMap::remove(int key)
void HashMap::increment(int key, int value)
```

Use write synchronization because they modify linked-list bucket state.

This prevents unsafe interleavings such as two threads modifying the same bucket chain at the same time or a reader traversing a bucket while another thread deletes an entry.

## Important Files

```text
rwlock.h
rwlock.cc
synch.cc
hashchain.cc
hashchain.h
Makefile
tests-phashfine
tests-phashfinerw
tests-nachos_sem
tests-nachos_lock
tests-nachos_rw
```

## Build and Run

Build the binaries:

```bash
make
```

Run all standard tests:

```bash
make test
```

Run tests for a specific implementation:

```bash
make test-phashfine
make test-phashfinerw
make test-nachos_sem
make test-nachos_lock
make test-nachos_rw
```

Run reference-output checks:

```bash
make check
```

Run an individual Pthreads hash map test:

```bash
./phashfinerw 200 1000 ALL_TESTS 3
./phashfine 200 1000 TEST_PUT 1
```

Run an individual Nachos synchronization test:

```bash
./nachos_sem -q -1
./nachos_lock -q 1
./nachos_rw -q 12
```

## Useful Compile-Time Flags

```text
NOLOCK        Disable synchronization.
P1_SEMAPHORE  Use semaphore-based synchronization.
P1_LOCK       Use lock-based synchronization.
P1_RWLOCK     Use reader-writer lock synchronization.
NACHOS        Enable Nachos-specific synchronization code.
P1BTESTS      Enable additional test code.
NOVALIDATE    Skip validation checks for performance testing.
NOYIELD       Disable inserted yields used to expose race conditions.
```

## My Contributions

My work focused on the synchronization implementation and debugging:

- Implemented writer-side reader-writer lock logic, including `startWrite()` and `doneWrite()`.
- Added private reader-writer lock state for active readers, active writers and waiting writers.
- Helped implement semaphore, lock and reader-writer lock integration in the hash map.
- Helped debug the Nachos synchronization path and the hash map concurrency tests.
- Contributed to validating the implementation with simple and comprehensive test suites.

## What This Demonstrates

This project demonstrates practical systems programming concepts:

- Mutual exclusion
- Race condition prevention
- Reader-writer synchronization
- Writer starvation prevention
- Condition-variable waiting and signaling
- Fine-grained locking
- Compile-time synchronization selection
- Debugging nondeterministic multithreaded behavior

## Notes

Some tests include deliberate thread yields to expose race conditions. Passing once does not guarantee a synchronization implementation is correct because concurrency bugs can be nondeterministic, so the tests should be run multiple times when debugging.
