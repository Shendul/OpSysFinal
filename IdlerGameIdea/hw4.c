/*
Original work Copyright (c) 2017 Anthony Leclerc <leclerca@cofc.edu>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
*/

#include <stdio.h>
#include <stdlib.h>  // for exit(), rand(), strtol()
#include <pthread.h>
#include <time.h>    // for nanosleep()
#include <errno.h>   // for EINTR error check in millisleep()

#include "binary_semaphore.h"

// you can adjust next two values to speedup/slowdown the simulation
#define MIN_SLEEP      20   // minimum sleep time in milliseconds
#define MAX_SLEEP     100   // maximum sleep time in milliseconds
//#define MIN_SLEEP       2   // minimum sleep time in milliseconds
//#define MAX_SLEEP      10   // maximum sleep time in milliseconds

#define START_SEED     17   // arbitrary value to seed random number generator

// guard_state with a value of k:
//          k < 0 : means guard is waiting in the room
//          k = 0 : means guard is in the hall of department
//          k > 0 : means guard is IN the room
int guard_state;         // waiting, in the hall, or in the room
int num_students;        // number of students in the room

// TODO:  list here the "handful" of semaphores you will need to synchronize
//        I've listed one you will need for sure, to "get you going"
binary_semaphore mutex;  // to protect shared variables (including semaphores)
binary_semaphore latch;  // one-way latch for students leaving the room
binary_semaphore cwait;  // for guard to wait to enter room
binary_semaphore clear;  // for guard to know when room is clear

// will malloc space for seeds[] in the main
unsigned int *seeds;     // rand seeds for guard and students generating delays

// NOTE:  globals below are initialized by command line args and never changed !
int capacity;       // maximum number of students in a room
int num_checks;     // number of checks the guard makes

inline void millisleep(long millisecs)   // delay for "millisecs" milliseconds
{ // details of this function are unimportant for the assignment
  struct timespec req;
  req.tv_sec  = millisecs / 1000;
  millisecs -= req.tv_sec * 1000;
  req.tv_nsec = millisecs * 1000000;
  while(nanosleep(&req, &req) == -1 && errno == EINTR);
}

// generate random int in range [min, max]
inline int rand_range(unsigned int *seedptr, long min, long max)
{ // details of this function are unimportant for the assignment
  // using reentrante version of rand() function (because multithreaded)
  // NOTE: however, overall behavior of code will still be non-deterministic
  return min + rand_r(seedptr) % (max - min + 1);
}

inline void study(long id)  // student studies for some random time
{ // details of this function are unimportant for the assignment
  int ms = rand_range(&seeds[id], MIN_SLEEP, MAX_SLEEP/2);
  printf("student %2ld studying in room with %2d students for %3d millisecs\n",
	 id, num_students, ms);
  millisleep(ms);
}

inline void do_something_else(long id)    // student does something else
{ // details of this function are unimportant for the assignment
  int ms = rand_range(&seeds[id], MIN_SLEEP, MAX_SLEEP);
  millisleep(ms);
}

inline void wait_to_enter()   // guard waits to enter room
{
  // NOTE:  we have (own) the mutex when we first enter this routine
  guard_state = -1;    // negative means waiting to enter room
  semSignalB(&mutex);  // release the mutex
  printf("\tguard waiting to enter room with %2d students...\n", num_students);
  semWaitB(&cwait);
  // NOTE: guard now "owns" mutex semaphore that "last" student waited on
  printf("\tguard done waiting to enter room with %2d students\n",
	 num_students);
}

inline void clearout_room()  // guard clears out the room
{
  // NOTE:  we have (own) the mutex when we first enter this routine
  guard_state = 1;     // positive means in the room
  printf("\tguard clearing out room with %2d students...\n",
	 num_students);
  semWaitB(&latch);    // setup the latch
  semSignalB(&mutex);  // release the mutex
  printf("\tguard waiting for students to clear out with %2d students...\n",
	 num_students);
  semWaitB(&clear);
  semSignalB(&latch);  // effectively removes the latch
  // NOTE:  we have (own) the mutex from last student leaving the study
  printf("\tguard done clearing out room\n");
}

inline void assess_security()  // guard assess room security
{
  // NOTE:  we have (own) the mutex when we first enter this routine
  guard_state = 1;     // positive means in the room
  int ms = rand_range(&seeds[0], MIN_SLEEP, MAX_SLEEP/2);
  printf("\tguard assessing room security for %3d millisecs...\n", ms);
  millisleep(ms);
  printf("\tguard done assessing room security\n");
}

inline void guard_walk_hallway()  // guard walks the hallway
{
  int ms = rand_range(&seeds[0], MIN_SLEEP, MAX_SLEEP/2);
  printf("\tguard walking the hallway for %3d millisecs...\n", ms);
  millisleep(ms);
}

// this function contains the main synchronization logic for the guard
inline void guard_check_room()
{
  semWaitB(&mutex);
  if (num_students > 0 && num_students < capacity) {
    // Two conditions can allow guard to enter: no students in the
    // room OR capacity students in the room.
    wait_to_enter();

    /*
      NOTE: at this point, either the room became empty OR the
      capacity-th student entered the room.  The two (similar)
      cases are described below:

      If empty, the "last" student to leave performed a
      semWaitB(mutex), but later, instead of performing a
      semSignalB(mutex), this "last" student performed a
      semSignalB(cwait) (see lines S6, S7 and S9 in function
      student_study_in_room).  The semSignalB(cwait) released the
      guard to be able to enter the room AND, since no
      semSignalB(mutex) was performed by the "last" student, the
      guard now has (owns) the mutex.

      If there are capacity now in the room, then (in a similar
      way as the empty room case above) the "last" student to have
      entered the room performed a semWaitB(mutex), but later, instead
      of performing a semSignalB(mutex), this "last" student performed
      a semSignalB(cwait) (see lines S1, S4 and S5 in function
      student_study_in_room).  The semSignalB(cwait) released the
      guard to be able to enter the room AND, since no
      semSignalB(mutex) was performed by the "last" student, the
      guard now has (owns) the mutex.
    */
  }

  if (num_students >= capacity)
    clearout_room();
  else
    assess_security();

  guard_state = 0;   // left room and is in the hallway
  printf("\tguard left room\n");
  semSignalB(&mutex);
}

// this function contains the main synchronization logic for a student
inline void student_study_in_room(long id)
{
  semWaitB(&mutex);       // S1:
  if (guard_state > 0) {  // ie. IN the room
    semSignalB(&mutex);
    semWaitB(&latch);     // S2
    semSignalB(&latch);   // S3
    semWaitB(&mutex);
  }

  num_students++;

  // guard_state < 0 means that the guard is waiting
  if (num_students == capacity && guard_state < 0) {
    printf("LAST student %2ld entering room with guard waiting\n", id);
    semSignalB(&cwait);   // S4:  NOTE semSignalB(&mutex) at S5 not performed
  }
  else
    semSignalB(&mutex);   // S5:

  study(id);

  semWaitB(&mutex);       // S6:
  num_students--;

  // NOTE:
  // guard_state < 0 means that the guard is waiting
  // guard_state > 0 means that the guard is IN the room

  if (num_students == 0 && guard_state < 0) {
    printf("LAST student %2ld left room with guard waiting\n", id);
    semSignalB(&cwait);   // S7:  NOTE semSignalB(&mutex) at S9 not performed
  }
  else if (num_students == 0 && guard_state > 0) {
    printf("LAST student %2ld left room with guard in it\n", id);
    semSignalB(&clear);   // S8:
  }
  else {
    printf("student %2ld left room\n", id);
    semSignalB(&mutex);   // S9:
  }
}

void* guard(void* arg)
{
  int i;
  srand(seeds[0]);

  for (i = 0; i < num_checks; i++) {
    guard_check_room();
    guard_walk_hallway();
  }

  return NULL;   // thread needs to return a void*
}

void* student(void* arg)
{
  long id = (long) arg;

  srand(seeds[id]);
  while (1) {
    student_study_in_room(id);
    do_something_else(id);
  }

  return NULL;   // thread needs to return a void*
}

int main(int argc, char** argv)
{
  int running = 1; // 1 is true, 0 is false.
  int cmd;
  int money = 0;
  while(running) {
    printf("Please Enter one of the commands below:\n(1) Get A report on current Money amount.\n(2) Exit the game.\n@ThreadIdler: ");
    scanf("%d", &cmd);
    if (cmd == 1){
      printf("Current Money: %d\n", money);
    } else if (cmd == 2) {
      exit(1);
    } else {
      printf("Invalid Command Entered\n");
    }
  }
}
