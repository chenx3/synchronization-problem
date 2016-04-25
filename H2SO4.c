/* Author: Tao Liu, Xi Chen */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

// functions of the 2 types of threads, one that produces oxygen and one hydrogen
void* oxygen(void*);
void* hydrogen(void*);
void* sulfur(void*);
void openSems();
void closeSems();
void delay(int);
int checkSem(sem_t*, char*);

// declare hydrogen semaphore as global variable
sem_t* hydro_sem;
sem_t* oxy_sem;
sem_t* hydro_leave_sem;
sem_t* hydro_full_sem;
sem_t* oxy_leave_sem;
sem_t* produce_sem;
sem_t* oxy_full_sem;

void openSems() {
  // create all the semaphores
  hydro_sem = sem_open("hydrosem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(hydro_sem, "hydrosem") == -1) {
    hydro_sem = sem_open("hydrosem", O_CREAT|O_EXCL, 0466, 0);
  }
  oxy_sem = sem_open("oxysem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(oxy_sem, "oxysem") == -1) {
    oxy_sem = sem_open("oxysem", O_CREAT|O_EXCL, 0466, 0);
  }
  hydro_leave_sem = sem_open("hydroleavesem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(hydro_leave_sem, "hydroleavesem") == -1) {
     hydro_leave_sem = sem_open("hydroleavesem", O_CREAT|O_EXCL, 0466, 0);
  }
  hydro_full_sem = sem_open("hydrofullsem", O_CREAT|O_EXCL, 0466, 1);
  while (checkSem(hydro_full_sem, "hydrofullsem") == -1) {
     hydro_full_sem = sem_open("hydrofullsem", O_CREAT|O_EXCL, 0466, 0);
  }
  oxy_leave_sem = sem_open("oxyleavesem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(oxy_leave_sem, "oxyleavesem") == -1) {
     oxy_leave_sem = sem_open("oxyleavesem", O_CREAT|O_EXCL, 0466, 0);
  }

  oxy_full_sem = sem_open("oxyfullsem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(oxy_full_sem, "oxyfullsem") == -1) {
     oxy_full_sem = sem_open("oxyfullsem", O_CREAT|O_EXCL, 0466, 0);
  }

  produce_sem = sem_open("producesem", O_CREAT|O_EXCL, 0466, 1);
  while (checkSem(produce_sem, "producesem") == -1) {
     produce_sem = sem_open("produce_sem", O_CREAT|O_EXCL, 0466, 1);
  }
 
}

void closeSems(){
  // close and unlink all the semaphores
  sem_close(hydro_sem);
  sem_unlink("hydrosem");
  sem_close(oxy_sem);
  sem_unlink("oxysem");
  sem_close(hydro_leave_sem);
  sem_unlink("hydroleavesem");
  sem_close(hydro_full_sem);
  sem_unlink("hydrofullsem");
  sem_close(oxy_leave_sem);
  sem_unlink("oxyleavesem");
  sem_close(produce_sem);
  sem_unlink("producesem");
  sem_close(oxy_full_sem);
  sem_unlink("oxyfullsem");
}

void* sulfur(void* args) {
  // produce a hydrogen molecule - takes some (small) random amount of time 
  delay(rand()%3000);
  printf("sulfur produced\n");
  fflush(stdout);
  
  // wait twice for hydrogen, wait 4 times for oxygen
  sem_wait(hydro_sem);
  //printf("get one hydro\n");
  //fflush(stdout);
  sem_wait(hydro_sem);
  //printf("get two hydro\n");
  //fflush(stdout);
  sem_wait(oxy_sem);
  //printf("get 1 oxy\n");
  //fflush(stdout);
  sem_wait(oxy_sem);
  //printf("get 2 oxy\n");
  //fflush(stdout);
  sem_wait(oxy_sem);
  //printf("get 3 oxy\n");
  //fflush(stdout);
  sem_wait(oxy_sem);
  //printf("get 4 oxy\n");
  //fflush(stdout);

  sem_wait(produce_sem);
  // produce H2SO4
  printf("produce H2SO4\n");
  fflush(stdout);
  
  // wake up hydrogen to leave
  sem_post(hydro_leave_sem);
  sem_post(hydro_leave_sem);
  
  // wait for all hydrogen to leave
  sem_wait(hydro_full_sem);
  sem_wait(hydro_full_sem);

  // sulfur leaves
  printf("sulfur leaving\n");
  fflush(stdout);
  
  // wake up oxygen to leave
  sem_post(oxy_leave_sem);
  sem_post(oxy_leave_sem);
  sem_post(oxy_leave_sem);
  sem_post(oxy_leave_sem);
  sem_wait(oxy_full_sem);
  sem_wait(oxy_full_sem);
  sem_wait(oxy_full_sem);
  sem_wait(oxy_full_sem);
  
  sem_post(produce_sem);
  return (void*) 0;
}


void* oxygen(void* args) {
  // produce an oxygen molecule - takes some (small) random amount of time 
  delay(rand()%2000);
  printf("oxygen produced\n");
  fflush(stdout);
  // signal oxy_sem 
  sem_post(oxy_sem);
  //printf("sign oxygen produced\n");
  // wait for sulfur to signal it to leave
  sem_wait(oxy_leave_sem);
  printf("oxygen leaving\n");
  fflush(stdout);
  sem_post(oxy_full_sem);
  return (void*) 0;
}

void* hydrogen(void* args) {
  // produce a hydrogen molecule - takes some (small) random amount of time 
  delay(rand()%3000);
  printf("hydrogen produced\n");
  fflush(stdout);

  sem_post(hydro_sem);

  // wait for sulfur to signal it to leave
  sem_wait(hydro_leave_sem);
  printf("hydrogen leaving\n");
  fflush(stdout);
  // signal sulfur that one hydrogen is left
  sem_post(hydro_full_sem);
  
  return (void*) 0;
}


/*
 * NOP function to simply use up CPU time
 * arg limit is number of times to run each loop, so runs limit^2 total loops
 */
void delay( int limit )
{
  int j, k;

  for( j=0; j < limit; j++ )
    {
      for( k=0; k < limit; k++ )
        {
        }
    }
}




int checkSem(sem_t* sema, char* filename) {
  if (sema==SEM_FAILED) {
    if (errno == EEXIST) {
      printf("semaphore %s already exists, unlinking and reopening\n", filename);
      fflush(stdout);
      sem_unlink(filename);
      return -1;
    }
    else {
      printf("semaphore %s could not be opened, error # %d\n", filename, errno);
      fflush(stdout);
      exit(1);
    }
  }
  return 0;
}
