/*
Original work Copyright (c) 2017 Nicholas Johnson and Spencer Wilder

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
#include <iostream>
using namespace std;
#include "binary_semaphore.h"

#define MIN_SLEEP      20   // minimum sleep time in milliseconds
#define MAX_SLEEP     100   // maximum sleep time in milliseconds
#define STANDARD_DELAY  200  // minimum sleep time in milliseconds

#define START_SEED     17   // arbitrary value to seed random number generator

binary_semaphore stash;  // to protect money variable

int money; // the main point of the game.

// will malloc space for seeds[] in the main
unsigned int *seeds;     // rand seed for generating delays

inline void millisleep(long millisecs) 
{ // delay for "millisecs" milliseconds
  struct timespec req;
  req.tv_sec  = millisecs / 1000;
  millisecs -= req.tv_sec * 1000;
  req.tv_nsec = millisecs * 1000000;
  while(nanosleep(&req, &req) == -1 && errno == EINTR);
}

// generate random int in range [min, max]
inline int rand_range(unsigned int *seedptr, long min, long max)
{ // using reentrante version of rand() function (because multithreaded)
  // NOTE: however, overall behavior of code will still be non-deterministic
  return min + rand_r(seedptr) % (max - min + 1);
}

inline void work(long id)  
{ // unit works for some random time
  int ms = rand_range(&seeds[id], MIN_SLEEP, MAX_SLEEP/2);
  printf("unit %2ld working for %3d millisecs\n",
   id, ms); // for testing purposes only, remove when finished
  millisleep(ms);
}

inline void do_something_else(long id)   
{ // a unit does something else
  int ms = rand_range(&seeds[id], MIN_SLEEP, MAX_SLEEP);
  millisleep(ms);
}

// this function contains the main synchronization logic for a farmer
inline void farmer_working(long id)
{
  // will need a semaphore that guards money amount so
  // that multiple threads can change the money without race condition.
  int work_for_min = 60;
  while(work_for_min > 0){
    money++;
    work_for_min--;
  }

}

void* farmer(void* arg)
{
  long id = (long) arg;
  char* status; // TODO use this to show if they are working ect.

  srand(seeds[id]);
  while (1) {
    farmer_working(id);
    do_something_else(id);
  }

  return NULL;   // thread needs to return a void*
}

int main(int argc, char** argv)
{
  //TODO make this dynamic or an array so we can have more farmers.
  pthread_t  fthread;      // farmer thread

  bool running = true; // 1 is true, 0 is false.
  int money = 0;
  int farmerPrice = 0; //TODO make this global
  int numFarmers = 0; // TODO make this better?
  while(running) {
    int cmd;
    printf("Please Enter one of the commands below:\n");
    millisleep(STANDARD_DELAY); // added delays for human readability
    printf("(1) Get A report on current Money amount.\n");
    millisleep(STANDARD_DELAY); // added delays for human readability
    printf("(2) Hire/Upgrade Menu.\n");
    millisleep(STANDARD_DELAY); // added delays for human readability
    printf("(10) Exit the game.\n");
    millisleep(STANDARD_DELAY); // added delays for human readability

    printf("@ThreadIdler: ");
    cin >> cmd;
    millisleep(STANDARD_DELAY); // added delays for human readability
    if (cmd == 1){
      printf("Current Money: %d\n", money);
      millisleep(STANDARD_DELAY); // added delays for human readability
    } else if (cmd == 2) {
      bool inHireMenu = true;
      while(inHireMenu){
        int hcmd;
        printf("Please Enter one of the commands below:\n");
        millisleep(STANDARD_DELAY); // added delays for human readability
        printf("(1) Hire a Farmer. Cost: %d\n", farmerPrice);
        millisleep(STANDARD_DELAY); // added delays for human readability
        cin >> hcmd;

        if(hcmd == 1){
          numFarmers++;
          pthread_create(&fthread, NULL, farmer, (void*) numFarmers);
        } else if (hcmd == 10){
          inHireMenu = false;
        } else {
          printf("Invalid Command Entered\n");
          millisleep(STANDARD_DELAY); // added delays for human readability
        }
      }
    } else if (cmd == 10) {
      running = false;
    } else {
      printf("Invalid Command Entered\n");
      millisleep(STANDARD_DELAY); // added delays for human readability
    }

  }
  return 0;
}