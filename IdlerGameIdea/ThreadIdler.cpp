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


#define STANDARD_DELAY  200  // delay for human readability

#define WORK_DELAY     20   // arbitrary value to seed random number generator

binary_semaphore stash;  // to protect money variable

int money; // the main point of the game.

inline void millisleep(long millisecs) 
{ // delay for "millisecs" milliseconds
  struct timespec req;
  req.tv_sec  = millisecs / 1000;
  millisecs -= req.tv_sec * 1000;
  req.tv_nsec = millisecs * 1000000;
  while(nanosleep(&req, &req) == -1 && errno == EINTR);
}

inline void work(long id)  
{ // unit works for some random time
  int ms = STANDARD_DELAY;
  millisleep(ms);
}

inline void do_something_else(long id)   
{ // a unit does something else
  int ms = STANDARD_DELAY;
  millisleep(ms);
}

// this function contains the main synchronization logic for a farmer
inline void farmer_working(long id)
{
  // will need a semaphore that guards money amount so
  // that multiple threads can change the money without race condition.
  int work_for_min = 60;
  while(work_for_min > 0){

    semWaitB(&stash);
    money++;
    semSignalB(&stash);

    work(id);
    //printf("unit %ld working. Current money: %d \n",
   //id, money); // for testing purposes only, remove when finished
    //printf("%d\n",work_for_min);
    work_for_min--;
  }

}

void* farmer(void* arg)
{
  long id = (long) arg;
  //char* status; // TODO use this to show if they are working ect.

  while (1) {
    farmer_working(id);
    do_something_else(id);
  }

  return NULL;   // thread needs to return a void*
}

inline void load (int fileToLoad){
  FILE *fp;
  char *fileName;

  if(fileToLoad == 1){
    fileName = "save1.txt";
  } else if (fileToLoad == 2){
    fileName = "save2.txt";
  }

  fp = fopen(fileName, "r");

  if (fp == NULL) {
  fprintf(stderr, "Can't open input file %s\n", fileName);
  exit(1);
}

  int loadedMoney;
  fscanf(fp, "%d", &loadedMoney);
  printf("loaded file: %s\n",fileName);
  money = loadedMoney;

}

int main(int argc, char** argv)
{

  //TODO make this dynamic or an array so we can have more farmers.
  pthread_t  fthread;      // farmer thread

  // init the semaphores
  semInitB(&stash, 1);


  bool running = true; // 1 is true, 0 is false.
  money = 0;
  int farmerPrice = 0; //TODO make this global
  int numFarmers = 0; // TODO make this better?

  int fileToLoad; // which slot to load from?
  //TODO maybe add in the ability to read in from a text file save.
  char *endptr1; // for strtol()
  fileToLoad = strtol(argv[1], &endptr1, 10);

  if (fileToLoad == 1){
    load(1);
  } else if (fileToLoad == 2){
    load(2);
  }


  while(running) {
    int cmd;
    printf("---------------------------------------\n");
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
      printf("---------------------------------------\n");
      printf("Current Money: %d\n", money);
      millisleep(STANDARD_DELAY); // added delays for human readability
    } else if (cmd == 2) {
      bool inHireMenu = true;
      while(inHireMenu){
        int hcmd;
        printf("---------------------------------------\n");
        printf("Please Enter one of the commands below:\n");
        millisleep(STANDARD_DELAY); // added delays for human readability
        printf("(1) Hire a Farmer. Cost: %d\n", farmerPrice);
        millisleep(STANDARD_DELAY); // added delays for human readability
        printf("(10) Exit Hire/Upgrade Menu.\n");
        millisleep(STANDARD_DELAY);
        printf("@ThreadIdler/HireMenu: ");
        cin >> hcmd;

        if(hcmd == 1){

          if(money >= farmerPrice){
            millisleep(STANDARD_DELAY);
            printf("---------------------------------------\n");
            millisleep(STANDARD_DELAY);
            // TODO make sure that money is semaphore guarded.
            printf("Farmer has been hired! money left: %d\n", money);
            millisleep(STANDARD_DELAY);
            numFarmers++;
            pthread_create(&fthread, NULL, farmer, (void*) 1);
          } else {
            millisleep(STANDARD_DELAY);
            printf("---------------------------------------\n");
            millisleep(STANDARD_DELAY);
            // TODO make sure that money is semaphore guarded.
            printf("Failed to hire farmer! need %d more money!\n", farmerPrice - money);
          }
        } else if (hcmd == 10){
          inHireMenu = false;
        } else {
          printf("Invalid Command Entered\n");
          millisleep(STANDARD_DELAY); // added delays for human readability
        }
      }
    } else if (cmd == 10) {
      running = false;
      printf("---------------------------------------\n");
      printf("        THANK YOU FOR PLAYING!!        \n");
      printf("---------------------------------------\n");
    } else {
      printf("Invalid Command Entered\n");
      millisleep(STANDARD_DELAY); // added delays for human readability
    }

  }
  return 0;
}