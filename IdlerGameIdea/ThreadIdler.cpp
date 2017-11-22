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
#define MAX_FARMERS 10 // max number of farmers that can be hired
#define WORK_DELAY     20   // arbitrary value to seed random number generator

//Define color coded text.
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m" // this is white

//Define Formatting
#define LINE_START "\t\t\t\t"
#define LINE_END  "\n\n"
#define DIVIDER "--------------------------------------------------------------------------------------\n"

binary_semaphore stash;  // to protect money variable

int money; // the main point of the game.
int numFarmers; // total number of farmers owned.
int farmerPrice; // cost to hire new farmer.

//TODO make this dynamic or an array so we can have more farmers.
pthread_t*  fthreads;      // farmer thread

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
  } else if (fileToLoad == 3){
    fileName = "save3.txt";
  }

  fp = fopen(fileName, "r");

  if (fp == NULL) {
  fprintf(stderr, "Can't open input file %s\n", fileName);
  exit(1);
}

  int loadedMoney;
  int loadedFarmers;
  fscanf(fp, "%d", &loadedMoney);
  fscanf(fp, "%d", &loadedFarmers);
  for (int f = 0; f < loadedFarmers; f++){
    //printf("this is a test, farmer loaded\n");
    numFarmers++;
    pthread_create(&fthreads[f], NULL, farmer, (void*) f);
    farmerPrice += 200; // make sure this matches what happens in hire menu.
  }
  money = loadedMoney;
  printf(LINE_START DIVIDER);
  printf(LINE_START LINE_START ANSI_COLOR_MAGENTA "loaded file: %s\n" ANSI_COLOR_RESET,fileName);

  fclose(fp);

}

inline void save (int fileToLoad){
  FILE *fp;
  char *fileName;

  if(fileToLoad == 1){
    fileName = "save1.txt";
  } else if (fileToLoad == 2){
    fileName = "save2.txt";
  } else if (fileToLoad == 3){
    fileName = "save3.txt";
  }

  fp = fopen(fileName, "w");

  if (fp == NULL) {
  fprintf(stderr, "Can't open input file %s\n", fileName);
  exit(1);
}
  semWaitB(&stash);
  int savedMoney = money;// get money
  semSignalB(&stash);

  int savedFarmers = numFarmers;// get money
  // money is first digit in file.
  fprintf(fp, "%d\n", savedMoney);
  // next digit is num of farmers
  fprintf(fp, "%d\n", savedFarmers);
  printf(LINE_START DIVIDER);
  printf(LINE_START LINE_START ANSI_COLOR_MAGENTA "Saved to file: %s\n" ANSI_COLOR_RESET,fileName);
  fclose(fp);

}

int main(int argc, char** argv)
{

  printf("\n\n\n\n\n\n\n\n\n\n");
  printf( LINE_START DIVIDER);
  printf( LINE_START LINE_START"WELCOME TO THREAD IDLER!\n");
  printf( LINE_START "\t\tPlease set your terminal to full screen for the best experience.\n");
  // init the semaphores
  semInitB(&stash, 1);

  fthreads = (pthread_t*) malloc(MAX_FARMERS*sizeof(pthread_t));


  bool running = true; // 1 is true, 0 is false.
  money = 0;
  farmerPrice = 0;
  numFarmers = 0;

  int fileToLoad; // which slot to load from?
  //TODO maybe add in the ability to read in from a text file save.
  char *endptr1; // for strtol()
  fileToLoad = strtol(argv[1], &endptr1, 10);

  if (fileToLoad == 1){
    load(1);
  } else if (fileToLoad == 2){
    load(2);
  } else if (fileToLoad == 3){
    load(3);
  }


  while(running) {
    int cmd;
    printf(LINE_START DIVIDER LINE_END);
    printf(LINE_START "Please Enter one of the commands below:" LINE_END);
    millisleep(STANDARD_DELAY); // added delays for human readability
    printf(LINE_START "(1) Get A report on current Money amount." LINE_END);
    millisleep(STANDARD_DELAY); // added delays for human readability
    printf(LINE_START "(2) Hire/Upgrade Menu." LINE_END);
    millisleep(STANDARD_DELAY); 
    // TODO add a command three that shows the status of all units owned.

    // TODO maybe add a reset command?
    printf(LINE_START "(9) Save Menu." LINE_END);
    millisleep(STANDARD_DELAY);
    printf(LINE_START "(10) Exit the game." LINE_END);
    millisleep(STANDARD_DELAY); // added delays for human readability

    printf(ANSI_COLOR_GREEN LINE_START "@ThreadIdler: " ANSI_COLOR_RESET);
    cin >> cmd;
    millisleep(STANDARD_DELAY); // added delays for human readability
    if (cmd == 1){
      printf(LINE_START DIVIDER);
      semWaitB(&stash);
      printf(ANSI_COLOR_YELLOW LINE_START  LINE_START "Current Money: %d\n" ANSI_COLOR_RESET, money);
      semSignalB(&stash);
      millisleep(STANDARD_DELAY); // added delays for human readability
    } else if (cmd == 2) {
      bool inHireMenu = true;
      while(inHireMenu){
        int hcmd;
        printf(LINE_START DIVIDER);
        printf(LINE_START "Please Enter one of the commands below:" LINE_END);
        millisleep(STANDARD_DELAY); // added delays for human readability
        if(numFarmers >= MAX_FARMERS){
          printf(LINE_START "Max amount of Farmers reached!" LINE_END);
          millisleep(STANDARD_DELAY); // added delays for human readability
        } else {
          printf(LINE_START "(1) Hire a Farmer. Cost:" ANSI_COLOR_YELLOW " %d" ANSI_COLOR_RESET LINE_END, farmerPrice);
          millisleep(STANDARD_DELAY); // added delays for human readability
        }
        // TODO add an upgrade for the farmers that will make them all make money faster or something.
        printf(LINE_START "(10) Exit Hire/Upgrade Menu." LINE_END);
        millisleep(STANDARD_DELAY);
        printf(ANSI_COLOR_CYAN LINE_START "@ThreadIdler/HireMenu: " ANSI_COLOR_RESET);
        cin >> hcmd;

        if(hcmd == 1){
          semWaitB(&stash);
          int moneySnapshot = money;
          semSignalB(&stash);
          if(moneySnapshot >= farmerPrice && numFarmers < MAX_FARMERS){


            millisleep(STANDARD_DELAY);
            printf(LINE_START DIVIDER);
            millisleep(STANDARD_DELAY);

            semWaitB(&stash);
            money -= farmerPrice;
            semSignalB(&stash);

            numFarmers++;
            pthread_create(&fthreads[numFarmers - 1], NULL, farmer, (void*) (numFarmers - 1));
            farmerPrice += 200;

            // TODO make sure that money is semaphore guarded.
            semWaitB(&stash);
            printf(LINE_START "Farmer has been hired! money left:" ANSI_COLOR_YELLOW " %d\n" ANSI_COLOR_RESET, money);
            semSignalB(&stash);
            millisleep(STANDARD_DELAY);
          } else {
            millisleep(STANDARD_DELAY);
            printf(LINE_START DIVIDER);
            millisleep(STANDARD_DELAY);
            // TODO make sure that money is semaphore guarded.
            if (numFarmers >= MAX_FARMERS){
              printf(LINE_START "You have already reached the maximum amount of Farmers!\n");
            } else {
              printf(LINE_START "Failed to hire farmer! need" ANSI_COLOR_YELLOW" %d" ANSI_COLOR_RESET " more money!\n", farmerPrice - moneySnapshot);
            }
          }
        } else if (hcmd == 10){
          inHireMenu = false;
        } else {
          printf(LINE_START "Invalid Command Entered\n");
          millisleep(STANDARD_DELAY); // added delays for human readability
        }
      }
    } else if (cmd == 9) {
      bool inSaveMenu = true;
      while(inSaveMenu){
        int scmd;
        printf(LINE_START DIVIDER);
        printf(LINE_START "Please Enter one of the commands below:" LINE_END);
        millisleep(STANDARD_DELAY); // added delays for human readability
        printf(LINE_START "(1) Save to file 1." LINE_END);
        millisleep(STANDARD_DELAY); // added delays for human readability
        printf(LINE_START "(2) Save to file 2." LINE_END);
        millisleep(STANDARD_DELAY);
        printf(LINE_START "(3) Save to file 3." LINE_END);
        millisleep(STANDARD_DELAY);
        printf(LINE_START "(10) Exit Save Menu." LINE_END);
        millisleep(STANDARD_DELAY);
        printf(ANSI_COLOR_MAGENTA LINE_START "@ThreadIdler/SaveMenu: " ANSI_COLOR_RESET);
        cin >> scmd;

        if (scmd == 1){
          save(1);
        } else if (scmd == 2){
          save(2);
        } else if (scmd == 3){
          save(3);
        } else if (scmd == 10){
          inSaveMenu = false;
        } else {
          printf(LINE_START "Invalid Command Entered\n");
          millisleep(STANDARD_DELAY);
        }
      }

    } else if (cmd == 10) {
      running = false;
      printf(LINE_START DIVIDER);
      printf(LINE_START "THANK YOU FOR PLAYING!!\n");
      printf(LINE_START DIVIDER);
    } else {
      printf(LINE_START "Invalid Command Entered\n");
      millisleep(STANDARD_DELAY); // added delays for human readability
    }

  }
  return 0;
}