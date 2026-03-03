#include <errno.h>
#include <fcntl.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define TOTAL_NUMBERS 30

int main() {
  srand(time(NULL));

  struct mq_attr attr;
  attr.mq_flags = O_NONBLOCK;
  attr.mq_maxmsg = 5;
  attr.mq_msgsize = sizeof(int);
  attr.mq_curmsgs = 0;

  mqd_t mq =
      mq_open("/validator_queue", O_CREAT | O_WRONLY | O_NONBLOCK, 0644, &attr);
  if (mq == -1) {
    perror("mq_open");
    return 1;
  }

  int sent = 0;
  while (sent < TOTAL_NUMBERS) {
    usleep(200000);
    int num = rand() % 500 + 1;
    if (mq_send(mq, (char *)&num, sizeof(int), 0) == -1) {
      if (errno == EAGAIN) {
        printf("waiting to send number...\n");
      } else {
        perror("mq_send");
        mq_close(mq);
        return 1;
      }
    } else {
      sent++;
      printf("Generator [%d/%d]: Sending number %d\n", sent, TOTAL_NUMBERS,
             num);
    }
  }

  mq_close(mq);
  return 0;
}