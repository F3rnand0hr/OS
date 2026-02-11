#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  pid_t pid = fork();
  if (pid < 0) {
    perror("fork");
    return EXIT_FAILURE;
  }
  if (pid == 0) {
    execlp("gcc", "gcc", "../tests/source_code.c", "-o", "../build/manager",
           (char *)NULL);
    fprintf(stderr, "execlp failed: %s\n", strerror(errno));
    _exit(EXIT_FAILURE);
  }

  /* Parent: wait for compilation to finish and check status */
  int status;
  pid_t w = waitpid(pid, &status, 0);
  if (w < 0) {
    perror("waitpid");
    return EXIT_FAILURE;
  }

  /* Ensure we're in build directory so ./manager resolves */
  if (chdir("../build") != 0) {
    perror("chdir");
    return EXIT_FAILURE;
  }

  pid_t children[2];
  for (int i = 0; i < 2; ++i) {
    pid_t c = fork();
    if (c < 0) {
      perror("fork");
      return EXIT_FAILURE;
    }
    if (c == 0) {
      pid_t mypid = getpid();
      char pid_str[32];
      snprintf(pid_str, sizeof(pid_str), "%d", (int)mypid);
      sleep(1);
      execlp("./manager", "manager", pid_str, (char *)NULL);
      fprintf(stderr, "execlp ./manager failed: %s\n", strerror(errno));
      _exit(EXIT_FAILURE);
    } else {
      children[i] = c;
    }
  }

  wait(NULL);
  wait(NULL);

  return EXIT_SUCCESS;
}