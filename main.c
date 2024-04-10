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
#define MESSAGE_LEN 255
#define INPUT_FILE_NAME_LEN 100
#define OUTPUT_FILE_NAME_LEN 100

/* --- Structs --- */

typedef struct thread_params_t {
  int pipe_file[2]; // [0] for read and [1] for write. use pipe for data
                    // transfer from thread A to thread B
  sem_t sem_A, sem_B, sem_C; // the semphore
  char message[MESSAGE_LEN];
  char input_file[INPUT_FILE_NAME_LEN];   // input file name
  char output_file[OUTPUT_FILE_NAME_LEN]; // output file name
} thread_params_t;

/* --- Global variables --- */

int sum = 1;
pthread_attr_t attr;
int shm_fd; // use shared memory for data transfer from thread B to Thread C

/* --- Prototypes --- */

/**
 * Initializes data and utilities used in thread params.
 */
void initialize_data(thread_params_t *params);

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
    return EXIT_FAILURE;
  }

  pthread_t tid[3]; // three threads
  thread_params_t params;

  // Initialization
  // TODO: Consider setting `inputFile` and `outputFile` in this procedure
  initialize_data(&params);
  // Set the `inputFile` parameter
  strncpy(params.input_file, argv[1], INPUT_FILE_NAME_LEN);
  // Set the `outputFile` parameter
  strncpy(params.output_file, argv[2], OUTPUT_FILE_NAME_LEN);

  // Create Threads
  pthread_create(&(tid[0]), &attr, &ThreadA, (void *)(&params));
  pthread_create(&(tid[1]), &attr, &ThreadB, (void *)(&params));
  pthread_create(&(tid[2]), &attr, &ThreadC, (void *)(&params));

  // Wait on threads to finish
  pthread_join(tid[0], NULL);
  pthread_join(tid[1], NULL);
  pthread_join(tid[2], NULL);

  return EXIT_SUCCESS;
}

void initialize_data(thread_params_t *params) {
  // Initialize Sempahores
  if (sem_init(&(params->sem_A), 0, 1) != 0) { // Set up Sem for thread A
    perror("error for init thread A");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&(params->sem_B), 0, 0) != 0) { // Set up Sem for thread B
    perror("error for init thread B");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&(params->sem_C), 0, 0) != 0) { // Set up Sem for thread C
    perror("error for init thread C");
    exit(EXIT_FAILURE);
  }

  // Initialize thread attributes
  pthread_attr_init(&attr);

  // TODO: add your code

  return;
}

void *ThreadA(void *params) {
  // Cast params to `thread_params_t`
  thread_params_t *my_params = params;
  // Wait for `sem_A` to acquire lock
  sem_wait(&my_params->sem_A);

  // TODO: Read data file line by line and write to pipe (for ThreadB)

  for (int i = 0; i < 5; i++) {
    sum = 2 * sum;
    printf("Thread A: sum = %d\n", sum);
  }

  // Pass onto `sem_B`
  sem_post(&my_params->sem_B);

  return NULL;
}

void *ThreadB(void *params) {
  // Cast params to `thread_params_t`
  thread_params_t *my_params = params;
  // Wait for `sem_B` to acquire lock
  sem_wait(&my_params->sem_B);

  // TODO: Read from pipe line by line and write to shared memory (for ThreadC)

  for (int i = 0; i < 3; i++) {
    sum = 3 * sum;
    printf("Thread B: sum = %d\n", sum);
  }

  // Pass onto `sem_C`
  sem_post(&my_params->sem_C);

  return NULL;
}

void *ThreadC(void *params) {
  // Cast params to `thread_params_t`
  thread_params_t *my_params = params;
  // Wait for `sem_C` to acquire lock
  sem_wait(&my_params->sem_C);

  // TODO: Read from shared memory line by line
  // TODO: Set flag to determine if reading from File Header or Content Region
  // TODO: If reading from Content Region, write to `&my_params->outputFile`

  for (int i = 0; i < 4; i++) {
    sum = sum - 5;
    printf("Thread C: Final sum = %d\n", sum);
  }

  // TODO: Is this call required?
  // Pass onto `sem_A`
  // sem_post(&my_params->sem_A);

  return NULL;
}
