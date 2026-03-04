#include <stdio.h>
#include "../include/functions.h" // include the functions.h file

volatile sig_atomic_t worker_done = 0; // global variable to check if the worker is done

void HandleSigUSR2(int sig) { // handle the SIGUSR2 signal
  (void)sig; // ignore the signal
  worker_done = 1; // set the worker_done variable to 1
}

int main() {
  srand(time(NULL)); // seed the random number generator 
  setbuf(stdout, NULL);

  struct sigaction sa; // struct to store the signal action
  sa.sa_handler = HandleSigUSR2; // set the signal handler to the HandleSigUSR2 function
  sigemptyset(&sa.sa_mask); // empty the signal mask
  sa.sa_flags = 0; // set the signal flags to 0
  sigaction(SIGUSR2, &sa, NULL); // set the signal action for the SIGUSR2 signal

  int fd_pids = shm_open(SHM_PIDS, O_CREAT | O_RDWR, 0666); // create a shared memory object for the pids, with parameters for the shared memory object
  ftruncate(fd_pids, sizeof(struct Registry)); // truncate the shared memory object to the size of the struct Registry
  struct Registry *registry = mmap(NULL, sizeof(struct Registry), // map the shared memory object to the process's address space
      PROT_READ | PROT_WRITE, MAP_SHARED, fd_pids, 0); // PROT_READ | PROT_WRITE: allow read and write access, MAP_SHARED: shared memory object is visible to all processes

  int fd_data = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666); // 0666: permissions for the shared memory object
  ftruncate(fd_data, sizeof(struct Batch)); // truncate the shared memory object to the size of the struct Batch
  struct Batch *data = mmap(NULL, sizeof(struct Batch), 
      PROT_READ | PROT_WRITE, MAP_SHARED, fd_data, 0);

  registry->generator_pid = getpid(); // get the process id of the generator
  registry->worker_pid = 0; // set the worker pid to 0

  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 10; j++)
      for (int k = 0; k < 10; k++)
        data->image[i][j][k] = rand() % 100; // generate a random number between 0 and 99

  pid_t pid = fork();
  if (pid == 0) { // if the process is the child process
    execlp("../../tests/classify", "classify", NULL); // execute the classify program
    perror("execlp"); // print the error message
    exit(1); // exit the program with status 1
  }
  waitpid(pid, NULL, 0); // wait for the child process to finish

  printf("[GENERATOR] Classification ready. Waiting Worker...\n"); // print the message

  while (registry->worker_pid == 0) { // wait for the worker to finish
    usleep(100000);
  }

  pid_t worker_pid = registry->worker_pid; // get the worker pid
  printf("[Generator] Sending SIGUSR1 to Worker (PID: %d)...\n", worker_pid);
  kill(worker_pid, SIGUSR1);

  while (!worker_done) { // wait for the worker to finish
    pause();
  }
  printf("[Generator] Worker finished.\n");

  munmap(registry, sizeof(struct Registry)); // unmap the shared memory object
  munmap(data, sizeof(struct Batch)); // unmap the shared memory object
  close(fd_pids); // close the shared memory object
  close(fd_data); // close the shared memory object
  shm_unlink(SHM_PIDS); // unlink the shared memory object
  shm_unlink(SHM_NAME); // unlink the shared memory object

  return 0; // return 0 to indicate success
}
