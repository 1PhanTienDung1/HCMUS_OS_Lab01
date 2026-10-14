#include "kernel/types.h"
#include "user/user.h"

const int MAX_LINE = 1000;

int
readLine(int fd, char *buf, int max)
{
  int n = 0;
  char c;
  int r;

  while (1) {
    r = read(fd, &c, sizeof(c));

    if (r < 0) {
      return r;
    }

    if (r == 0) {
      if (n == 0) {
        return 0;
      }
      break;
    }

    if (c == '\n') {
      break;
    }

    if (n + 1 < max) {
      buf[n++] = c;
    }
  }

  buf[n] = '\0';

  return 1;
}

int
main(int argc, char *argv[])
{
  char *f1;
  char *f2;
  int q = 0;

  if (argc == 4 && strcmp(argv[1], "-q") == 0) {
    q = 1;
    f1 = argv[2];
    f2 = argv[3];
  } else if (argc == 4 && strcmp(argv[3], "-q") == 0) {
    q = 1;
    f1 = argv[1];
    f2 = argv[2];
  } else if (argc == 3) {
    f1 = argv[1];
    f2 = argv[2];
  } else {
    printf("usage: diff [-q] file1 file2\n");
    exit(1);
  }

  int fd1 = open(f1, 0);
  if (fd1 < 0) {
    printf("diff: cannot open %s\n", f1);
    exit(1);
  }

  int fd2 = open(f2, 0);
  if (fd2 < 0) {
    printf("diff: cannot open %s\n", f2);
    close(fd1);
    exit(1);
  }

  char line1[MAX_LINE];
  char line2[MAX_LINE];
  int r1, r2;
  int cnt = 0;

  while (1) {
    r1 = readLine(fd1, line1, sizeof(line1));
    r2 = readLine(fd2, line2, sizeof(line2));
    // printf("%d %d\n", r1, r2);

    if (r1 == 0 && r2 == 0) {
      break;
    }

    if (r1 < 0 || r2 < 0) {
      printf("diff: read error\n");
      close(fd1);
      close(fd2);
      exit(1);
    }

    ++cnt;
    if (r1 == 0 || r2 == 0) {
      if (q) {
        printf("diff: files differ\n");
        break;
      }

      if (r1 == 0) {
        printf("%s:%d: < EOF\n", f1, cnt);
        printf("%s:%d: > %s\n", f2, cnt, line2);
      } else {
        printf("%s:%d: < %s\n", f1, cnt, line1);
        printf("%s:%d: > EOF\n", f2, cnt);
      }

      continue;
    }

    if (strcmp(line1, line2) != 0) {
      if (q) {
        printf("diff: files differ\n");
        break;
      }

      printf("%s:%d: < %s\n", f1, cnt, line1);
      printf("%s:%d: > %s\n", f2, cnt, line2);
    }
  }

  close(fd1);
  close(fd2);

  return 0;
}