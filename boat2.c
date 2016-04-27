#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

// define all the global variables

// child counts as 1, adult counts as 2
int boat = 0;
// 0 represent Oahu, 1 represent Molokai
int boat_location = 0;
int child_on_Oahu = 0;
int child_on_Molokai = 0;
int adult_on_Oahu = 0;
int adult_on_Molokai = 0;
int total_people = 0;
int arrive = 0;
// define all the condition variables and semiphores
pthread_mutex_t lock_mutex;
pthread_cond_t wait_on_O;
pthread_cond_t wait_on_M;
pthread_cond_t boat_full;
pthread_cond_t boat_arrive;
sem_t* arrive_sem;
sem_t* ready_sem;
sem_t* leave_sem;

void* child(void*);
void* adult(void*);
void* initSynch();
void* closeSynch();
void shuffle(int* intArray, int arrayLen);
int checkSem(sem_t* sema, char* filename);

int main(int argc, char* argv[]){
  closeSynch();
  if (argc < 3) {
    printf("\nmust include num of children and adult as command line arguments (e.g. './a.out 4 2')\n\n");
    exit(1);
  }
  int childrennum = atoi(argv[1]);
  int adultnum = atoi(argv[2]);
  int total = childrennum + adultnum;

  // initialize all the locks, condition-vars, and semaphores
  initSynch();
  
  // seed the random number generator with the current time
  srand(time(NULL));

  // add desired number of each type of atom (represented as a simple int) to an array
  // oxygen is represented as 1, hydrogen as 2, and sulfur as 3
  int order[total];
  int i;
  for (i=0; i<childrennum; i++) {
    order[i] = 1;
  }
  for (; i<total; i++) {
    order[i] = 2;
  }

  // order now has # of 1's, 2's, and 3's to reflect # of 3 types of atoms,
  // so just need to shuffle to get random order 
  shuffle(order, total);

  // now create threads in shuffled order 
  pthread_t human[total];
  for (i=0; i<total; i++) {
    if (order[i]==1) pthread_create(&human[i], NULL, child, NULL);
    else if (order[i]==2) pthread_create(&human[i], NULL, adult, NULL);
    else printf("something went horribly wrong!!!\n");
  }

  // wait for all the threads to start
  for (i=0;i<total;i++){
    sem_wait(arrive_sem);
  }
    // wait for all the threads to start
  for (i=0;i<total;i++){
    sem_post(ready_sem);
  }

  // wait for all people have arrived on Molokai before exiting
  sem_wait(leave_sem);
  
   // join all threads before letting main exit
  for (i=0; i<total; i++) {
    pthread_join(human[i], NULL);
  }
  
  closeSynch();
  
}

void* initSynch(){
  // create all the semaphores
  arrive_sem = sem_open("arrivesem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(arrive_sem, "arrivesem") == -1) {
    arrive_sem = sem_open("arrivesem", O_CREAT|O_EXCL, 0466, 0);
  }
  ready_sem = sem_open("readysem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(ready_sem, "readysem") == -1) {
    ready_sem = sem_open("readysem", O_CREAT|O_EXCL, 0466, 0);
  }
  leave_sem = sem_open("leavesem", O_CREAT|O_EXCL, 0466, 0);
  while (checkSem(leave_sem, "leavesem") == -1) {
     leave_sem = sem_open("leavesem", O_CREAT|O_EXCL, 0466, 0);
  }
  
  // create all the condition variables and lock
  pthread_mutex_init(&lock_mutex, NULL);
  pthread_cond_init (&wait_on_O, NULL);
  pthread_cond_init (&wait_on_M, NULL);
  pthread_cond_init (&boat_full, NULL);
  pthread_cond_init (&boat_arrive, NULL);
  return (void*) 0;
}

void* closeSynch(){
  sem_close(arrive_sem);
  sem_unlink("arrivesem");
  sem_close(ready_sem);
  sem_unlink("readysem");
  sem_close(leave_sem);
  sem_unlink("leavesem");
  pthread_mutex_destroy(&lock_mutex);
  pthread_cond_destroy(&wait_on_O);
  pthread_cond_destroy(&wait_on_O);
  pthread_cond_destroy(&wait_on_M);
  pthread_cond_destroy(&boat_full);
  pthread_cond_destroy(&boat_arrive);
  return (void*) 0;
}


void shuffle(int* intArray, int arrayLen) {
  int i=0;
  for (i=0; i<arrayLen; i++) {
    int r = rand()%arrayLen;
    int temp = intArray[i];
    intArray[i] = intArray[r];
    intArray[r] = temp;
  }
}

void* child(void* args){
  printf("Child show up on Oahu\n");
  fflush(stdout);
  // init variables
  int location = 0;
  total_people = total_people + 1;
  child_on_Oahu = child_on_Oahu + 1;
  // wait for everyone get ready
  sem_post(arrive_sem);
  sem_wait(ready_sem);
  while(1){
    pthread_mutex_lock(&lock_mutex);
    // if the child is in Oahu
    if(location == 0){
    
      // if the boat is not in oahu, full, or only one child and multiple adults on Oahu, wait
      while (boat_location == 1|| boat == 2 || (child_on_Oahu == 1 && adult_on_Oahu > 0)){
        pthread_cond_wait(&wait_on_O, &lock_mutex);
      }
      
     // if only on child on Oahu, row to Molokai
      if(child_on_Oahu == 1 && boat == 0){
        printf("Child getting the boat on Oahu\n");
        fflush(stdout);
        printf("Child rowing the boat from Oahu to Molokai\n");
        fflush(stdout);
        location = 1;
        boat_location = 1;
        child_on_Molokai = child_on_Molokai + 1;
        child_on_Oahu = child_on_Oahu - 1;
        printf("Child getting off the boat on Molokai\n");
        fflush(stdout);
        // if more than one child on Oahu
      }else{
        // child getting into the boat
        printf("Child getting into the boat on Oahu\n");
        fflush(stdout);
        boat = boat + 1;
        // if there is two children on the boat, row to Molokai
        if (boat == 2){
          // signal the passenger the boat is full
          pthread_cond_signal(&boat_full);
          // rowing to Malakai
          printf("Child rowing the boat from Oahu to Malakai\n");
          fflush(stdout);
          // set the child's location and the boat's location
          location = 1;
          boat_location = 1;
          child_on_Molokai = child_on_Molokai + 2;
          child_on_Oahu = child_on_Oahu - 2;
          printf("Child getting off the boat in Malakai\n");
          fflush(stdout);
          boat = boat - 2;
          // wait up all people waiting on Molokai
          pthread_cond_broadcast(&wait_on_M);
        }else{
          // waiting for a second passenger
          pthread_cond_wait(&boat_full,&lock_mutex);
          printf("Child riding the boat from Oahu to Malakai\n");
          fflush(stdout);
          printf("Child getting off the boat in Malakai\n");
          fflush(stdout);
          location = 1;
        }
      }
    }else{
      // the child is in Molakai
      if (adult_on_Molokai + child_on_Molokai == total_people){
        sem_post(leave_sem);
        pthread_mutex_unlock(&lock_mutex);
        pthread_exit(0);
      }
      // if the boat is not in oahu, full, or only one child and multiple adults on Oahu, wait
      while (boat_location == 0 || boat == 1 ){
        pthread_cond_wait(&wait_on_M, &lock_mutex);
      }
      if (adult_on_Molokai + child_on_Molokai == total_people){
        sem_post(leave_sem);
        pthread_mutex_unlock(&lock_mutex);
        pthread_exit(0);
      }
      // rowing the boat to Molakai
      printf("Child getting the boat on Molokai\n");
      fflush(stdout);
      printf("Child rowing the boat from Molokai to Oahu\n");
      fflush(stdout);
      location = 0;
      boat_location = 0;
      child_on_Molokai = child_on_Molokai - 1;
      child_on_Oahu = child_on_Oahu + 1;
      printf("Child getting off the boat in Oahu\n");
      fflush(stdout);
      // wake up all people on Oahu
      pthread_cond_broadcast(&wait_on_O);
    }
    pthread_mutex_unlock(&lock_mutex);
  }
}



void* adult(void* args){
  printf("adult show up on Oahu\n");
  fflush(stdout);
  
  // init variables
  int location = 0;
  total_people = total_people + 1;
  adult_on_Oahu = adult_on_Oahu + 1;
  
  // wait for everyone get ready
  sem_post(arrive_sem);
  sem_wait(ready_sem);

  while(1){
    pthread_mutex_lock(&lock_mutex);
    if (location == 0){
      // wait on Oahu if boat if full or there are more than 1 child on Oahu or boat is in Molokai
      while (boat != 0 || child_on_Oahu > 1 || boat_location == 1){
        pthread_cond_wait(&wait_on_O, &lock_mutex);
      }
      // row to Molokai
      printf("Adult getting into the boat on Oahu\n");
      fflush(stdout);
      printf("Adult rowing boat from Oahu to Molokai\n");
      fflush(stdout);
      location = 1;
      boat_location = 1;
      adult_on_Molokai = adult_on_Molokai + 1;
      adult_on_Oahu = adult_on_Oahu - 1;
      printf("Adult arrives at Molokai\n");
      fflush(stdout);

      // wake up all people waiting on Molokai
      pthread_cond_broadcast(&wait_on_M);

    // wait on Molokai if the adult is on Molokai
    }else{
      // if all the people are in Molokai, exit
      if (adult_on_Molokai + child_on_Molokai == total_people){
        pthread_mutex_unlock(&lock_mutex);
        pthread_exit(0);
      }
      pthread_cond_wait(&wait_on_M, &lock_mutex);
    }
    // if all the people are in Molokai, exit
    if (adult_on_Molokai + child_on_Molokai == total_people){
      pthread_mutex_unlock(&lock_mutex);
      pthread_exit(0);
    }
    pthread_mutex_unlock(&lock_mutex);
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
