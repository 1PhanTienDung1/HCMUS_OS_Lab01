#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(2, "usage: sleep ticks\n");
    exit(1);
  }
  uint64 value = 0;
  if (argv[1][0] == '\0') {
    fprintf(2, "sleep: ticks must be a non-negative integer\n");
    exit(1);
  }
  for (int i = 0; argv[1][i] != '\0'; ++i) {
    if (argv[1][i] < '0' || argv[1][i] > '9') {
      fprintf(2, "sleep: ticks must be a non-negative integer\n");
      exit(1);
    }
    value = value * 10 + (argv[1][i] - '0');
    if (value > 2147483647) {
      fprintf(2, "sleep: ticks is too large\n");
      exit(1);
    }
  }

  int ticks = atoi(argv[1]);

  if (pause(ticks) == -1) {
    fprintf(2, "sleep: wait interrupted\n");
    exit(1);
  }

  exit(0);
}
