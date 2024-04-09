/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology
// Sydney(UTS) Please complete the code based on the assignment requirement.

//***********************************************************************************
/***********************************************************************************/

/*
  To compile and run this program ensure that gcc is installed and run the
  following commands:

    $ gcc main.c -o ./out/a2 -lpthread -lrt -Wall
    $ ./out/a2 data.txt output.txt

  If you have `make` installed, you may prefer to run the following instead:

    $ make
    $ ./out/a2 data.txt output.txt
*/

#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* To be used for your memory allocation, write/read. man mmsp */
#define SHARED_MEM_NAME "/my_shared_memory"
#define SHARED_MEM_SIZE 1024

/* --- Structs --- */
typedef struct ThreadParams {
  int pipeFile[2]; // [0] for read and [1] for write. use pipe for data transfer
                   // from thread A to thread B
  sem_t sem_A, sem_B, sem_C; // the semphore
  char message[255];
  char inputFile[100];  // input file name
  char outputFile[100]; // output file name
} ThreadParams;

/* --- Global variables --- */
int sum = 1;

pthread_attr_t attr;

int shm_fd; // use shared memory for data transfer from thread B to Thread C

/* --- Prototypes --- */

/**
 * Initializes data and utilities used in thread params.
 */
void initializeData(ThreadParams *params);

/**
 * This thread reads data from `data.txt` and writes each line to a pipe
 */
void *ThreadA(void *params);

/**
 * This thread reads data from pipe used in ThreadA and writes it to a shared
 * variable.
 */
void *ThreadB(void *params);

/**
 * This thread reads from shared variable and outputs non-header text to
 * `src.txt`.
 */
void *ThreadC(void *params);

/** --- Main code --- */
int main(int argc, char const *argv[]) {
  // Verify the data and output file name are provided as arguments.
  // NOTE: The program executable (`./out/a2`) counts as an argument too, so
  // `argc` should equal to 3.
  if (argc != 3) {
    fprintf(stderr, "USAGE: ./out/a2 data.txt output.txt\n");
    return 1;
  }

  pthread_t tid[3]; // three threads
  ThreadParams params;

  // Initialization
  initializeData(&params);

  // Create Threads
  pthread_create(&(tid[0]), &attr, &ThreadA, (void *)(&params));

  // TODO: add your code

  // Wait on threads to finish
  pthread_join(tid[0], NULL);

  // TODO: add your code

  return 0;
}

void initializeData(ThreadParams *params) {
  // Initialize Sempahores
  if (sem_init(&(params->sem_A), 0, 1) != 0) { // Set up Sem for thread A
    perror("error for init thread A");
    exit(1);
  }
  if (sem_init(&(params->sem_B), 0, 0) != 0) { // Set up Sem for thread B
    perror("error for init thread B");
    exit(1);
  }
  if (sem_init(&(params->sem_C), 0, 0) != 0) { // Set up Sem for thread C
    perror("error for init thread C");
    exit(1);
  }

  // Initialize thread attributes
  pthread_attr_init(&attr);
  // TODO: add your code

  return;
}

void *ThreadA(void *params) {
  // TODO: add your code

  printf("Thread A: sum = %d\n", sum);
}

void *ThreadB(void *params) {
  // TODO: add your code

  printf("Thread B: sum = %d\n", sum);
}

void *ThreadC(void *params) {
  // TODO: add your code

  printf("Thread C: Final sum = %d\n", sum);
}
