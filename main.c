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
#include <sys/mman.h> // TODO: This was not part of the template
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

/* To be used for your memory allocation, write/read. man mmsp */
#define SHARED_MEM_NAME "/my_shared_memory"
#define SHARED_MEM_SIZE 1024

/** Set to 1 to turn on DEBUG, otherwise set to 0. */
#define DEBUG 0
/** The maximum string length of the message to pass by shared memory. */
#define MESSAGE_LEN 255
/** The maximum string length of the input file name. */
#define INPUT_FILE_NAME_LEN 100
/** The maximum string length of the output file name. */
#define OUTPUT_FILE_NAME_LEN 100
/** The substring to match that indicates the end of the File Header Region. */
#define END_HEADER_SUBSTRING "end_header\n"
/** The substring length of `END_HEADER_SUBSTRING`. */
#define END_HEADER_SUBSTRING_LEN strlen(END_HEADER_SUBSTRING)

/* --- Structs --- */

// TODO: Is it okay to convert this (and field members) to snake case?
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
 * TODO: Is it okay to convert this to snake case?
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
 * src.txt.
 */
void *ThreadC(void *params);

/* --- Main code --- */

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
  initialize_data(&params);
  // Set the `input_file` parameter
  strncpy(params.input_file, argv[1], INPUT_FILE_NAME_LEN);
  // Set the `output_file` parameter
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

  // Initialize the pipe
  int pipe_result = pipe(params->pipe_file);
  if (pipe_result < 0) {
    perror("Failed to pipe");
    exit(EXIT_FAILURE);
  }

  return;
}

void *ThreadA(void *params) {
  // Cast params to `thread_params_t`
  thread_params_t *my_params = params;
  // Wait for `sem_A` to acquire lock
  sem_wait(&my_params->sem_A);

  FILE *data_file_ptr;  // Data file
  char write_data[100]; // Current line of file to write to pipe
  ssize_t write_len;    // The length of the current line written to the pipe

  // Attempt to open the data file
  if ((data_file_ptr = fopen(my_params->input_file, "r")) == NULL) {
    perror("Failed to open data file");
    exit(EXIT_FAILURE);
  }

  // Read data file line by line and write to pipe (for ThreadB)
  while (fgets(write_data, sizeof(write_data), data_file_ptr) != NULL) {
#if DEBUG
    printf("[A] Write to pipe: %s", write_data);
#endif
    write_len = write(my_params->pipe_file[1], write_data, strlen(write_data));
    if (write_len < 0) {
      perror("Failed to write to pipe");
      exit(EXIT_FAILURE);
    }
  }

  // Close the pipe after writing to it
  close(my_params->pipe_file[1]);

  for (int i = 0; i < 5; i++) {
    sum = 2 * sum;
    printf("Thread A: sum = %d\n", sum);
  }

  // Close the file
  fclose(data_file_ptr);
  // Pass onto `sem_B`
  sem_post(&my_params->sem_B);

  return NULL;
}

void *ThreadB(void *params) {
  // Cast params to `thread_params_t`
  thread_params_t *my_params = params;
  // Wait for `sem_B` to acquire lock
  sem_wait(&my_params->sem_B);

  // Current buffer of text read from the pipe (from ThreadA)
  // NOTE: This is NOT guaranteed to hold a single line of text as only a 100
  // characters will be read on each loop.
  char read_data[100];
  // The length of the string read from the pipe. This may be negative if the
  // `read()` call fails.
  ssize_t read_len;

  // Create and open a new shared memory object
  shm_fd = shm_open(SHARED_MEM_NAME, O_CREAT | O_RDWR, 0666);
  // Configure the size of the shared memory object
  ftruncate(shm_fd, SHARED_MEM_SIZE);
  // Create a new mapping of the shared memory object in virtual address space
  void *shm_ptr = mmap(0, SHARED_MEM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

  // Continuously read from pipe and write to shared memory (for ThreadC)
  do {
    // Read the message from the pipe (from ThreadA)
    read_len = read(my_params->pipe_file[0], read_data, sizeof(read_data));
    if (read_len < 0) {
      perror("Failed to read from pipe");
      exit(EXIT_FAILURE);
    }

    // Fail-safe in case there was no message in the pipe to begin with
    if (read_len == 0) {
#if DEBUG
      printf("[B] End of message from pipe\n");
#endif
      break;
    }

    // Since the length of `read_data` is currently equal to its capacity (100),
    // the resulting string will either be fully replaced if 100 characters are
    // read, or will be partially replaced in the case of the last iteration.
    // To ensure the string is properly terminated, a NULL terminator will be
    // manually placed here.
    read_data[read_len] = '\0';
#if DEBUG
    printf("[B] Read from pipe (%zd bytes):\n%s\n", read_len, read_data);
#endif

    // Append the data from the pipe to the shared memory pointer
    sprintf(shm_ptr + strlen(shm_ptr), "%s", read_data);
  } while (read_len > 0);

  for (int i = 0; i < 3; i++) {
    sum = 3 * sum;
    printf("Thread B: sum = %d\n", sum);
  }

  // Close pipe
  // TODO: Is this call required?
  // close(my_params->pipe_file[0]);

  // Pass onto `sem_C`
  sem_post(&my_params->sem_C);

  return NULL;
}

void *ThreadC(void *params) {
  // Cast params to `thread_params_t`
  thread_params_t *my_params = params;
  // Wait for `sem_C` to acquire lock
  sem_wait(&my_params->sem_C);

  // Create a new mapping of the shared memory object in virtual address space
  void *shm_ptr = mmap(0, SHARED_MEM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
#if DEBUG
  printf("[C] Read from shared memory object:\n%s\n", (char *)shm_ptr);
#endif

  // Look for `END_HEADER_SUBSTRING` to know where the Content Region starts
  char *end_header_offset = strstr((char *)shm_ptr, END_HEADER_SUBSTRING);
  if (end_header_offset == NULL) {
    // Abort the program if no `END_HEADER_SUBSTRING` is found - the data file
    // may be malformed or does not follow the expected structure
    fprintf(stderr, "File Header is not present in the provided data file\n");
    exit(EXIT_FAILURE);
  }

  char *content_region_offset = end_header_offset + END_HEADER_SUBSTRING_LEN;
#if DEBUG
  printf("[C] Content region:\n%s\n", content_region_offset);
#endif

  FILE *output_file_ptr; // Output file
  if ((output_file_ptr = fopen(my_params->output_file, "w")) == NULL) {
    perror("Failed to create/open output file");
    exit(EXIT_FAILURE);
  }

  // Write the Content Region to the output file. The file contents will be
  // replaced.
  int bytes_written = fprintf(output_file_ptr, "%s", content_region_offset);
#if DEBUG
  printf("[C] Successfully wrote %d bytes to '%s'\n", bytes_written,
         my_params->output_file);
#else
  // This is a no-op. The purpose of this line is to supress GCC from issuing an
  // "unused variable" warning when `DEBUG` is false.
  (void)bytes_written;
#endif

  for (int i = 0; i < 4; i++) {
    sum = sum - 5;
    printf("Thread C: Final sum = %d\n", sum);
  }

  // Unlink shared memory
  shm_unlink(SHARED_MEM_NAME);

  // Pass onto `sem_A`
  sem_post(&my_params->sem_A);

  return NULL;
}
