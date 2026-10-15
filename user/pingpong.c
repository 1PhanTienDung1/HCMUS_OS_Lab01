#include "kernel/types.h"
#include "user/user.h"

static void
fail(const char *operation)
{
  fprintf(2, "pingpong: %s failed\n", operation);
  exit(1);
}

int
main(void)
{
  // 2 pipes
  int to_child[2];
  int to_parent[2];
  char byte = 'o';
  // create 2 pipes
  if (pipe(to_child) == -1 || pipe(to_parent) == -1)
    fail("pipe");
  // only proceed if creating successfully
  int pid = fork();
  if (pid == -1)
    fail("fork");

  if (pid == 0) {
    close(to_child[1]);
    close(to_parent[0]);

    if (read(to_child[0], &byte, 1) != 1) {
      fail("read ping");
    }
    close(to_child[0]);

    printf("%d: received ping\n", getpid());

    if (write(to_parent[1], &byte, 1) != 1)
      fail("write pong");
    close(to_parent[1]);

    exit(0);
  } else {
    close(to_child[0]);
    close(to_parent[1]);

    if (write(to_child[1], &byte, 1) != 1)
      fail("write ping");
    close(to_child[1]);

    if (read(to_parent[0], &byte, 1) != 1)
      fail("read pong");
    close(to_parent[0]);

    printf("%d: received pong\n", getpid());
    int status;
    if (wait(&status) < 0)
      fail("wait");
    if (status != 0)
      exit(1);
    exit(0);
  }
}
