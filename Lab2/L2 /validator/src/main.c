#include <fcntl.h>
#include <mqueue.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define TOTAL_NUMBERS 30

bool IsPrime(int n) {
  if (n <= 1)
    return false;
  for (int i = 2; i * i <= n; i++) {
    if (n % i == 0)
      return false;
  }
  return true;
}

int main() {
  mqd_t mq = mq_open("/validator_queue", O_RDONLY);
  if (mq == (mqd_t)-1) {
    perror("mq_open");
    return 1;
  }

  int primes[30], non_primes[30];
  int p_count = 0, np_count = 0;

  for (int i = 0; i < TOTAL_NUMBERS; i++) {
    int num;
    mq_receive(mq, (char *)&num, sizeof(int), NULL);

    if (IsPrime(num)) {
      primes[p_count++] = num;
    } else {
      non_primes[np_count++] = num;
    }
    // 500ms
    usleep(500000);
  }

  mq_close(mq);

  printf("Total processed: %d\n", TOTAL_NUMBERS);
  printf("primes: %d | non primes: %d\n", p_count, np_count);

  printf("Primes numbers: ");
  for (int i = 0; i < p_count; i++) {
    printf("%d", primes[i]);
    if (i < p_count - 1)
      printf(",");
  }
  printf("\n");

  printf("Non primes numbers: ");
  for (int i = 0; i < np_count; i++) {
    printf("%d", non_primes[i]);
    if (i < np_count - 1)
      printf(",");
  }
  printf("\n");

  return 0;
}