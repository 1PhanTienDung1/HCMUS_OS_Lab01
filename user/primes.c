#include "kernel/types.h"
#include "user.h"

// void primes(int) __attribute__((noreturn));
void
primes(int read_fd)
{
  while (1) {
    int p;
    if (read(read_fd, &p, sizeof(p)) == 0) {
      close(read_fd);
      exit(0);
    }

    printf("prime %d\n", p);

    int fd[2];
    pipe(fd);

    int pid = fork();

    if (pid == 0) {
      close(fd[1]);
      close(read_fd);

      read_fd = fd[0];
      continue;
    } else {
      close(fd[0]);

      int n;
      while (read(read_fd, &n, sizeof(n)) > 0) {
        if (n % p == 0)
          continue;
        write(fd[1], &n, sizeof(n));
      }

      close(fd[1]);
      wait(0);
      exit(0);
    }
  }
}

int
main()
{
  int fd[2];
  pipe(fd);

  int pid = fork();

  if (pid == 0) {
    close(fd[1]);
    primes(fd[0]);
  } else {
    close(fd[0]);

    for (int i = 2; i <= 280; ++i) {
      write(fd[1], &i, sizeof(i));
    }

    close(fd[1]);
    wait(0);
  }

  return 0;
}
