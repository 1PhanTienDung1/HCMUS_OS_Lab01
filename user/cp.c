#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(2, "usage: cp src dst\n");
    exit(1);
  }

  int src = open(argv[1], O_RDONLY);
  if (src == -1) {
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  struct stat src_info;
  if (fstat(src, &src_info) == -1 || src_info.type != T_FILE) {
    fprintf(2, "cp: source must be a regular file\n");
    close(src);
    exit(1);
  }

  // a = b
  struct stat dst_info;
  if (stat(argv[2], &dst_info) == 0 && src_info.dev == dst_info.dev &&
      src_info.ino == dst_info.ino) {
    fprintf(2, "cp: source and destination are the same file\n");
    close(src);
    exit(1);
  }

  int dst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC);
  if (dst == -1) {
    fprintf(2, "cp: cannot open %s\n", argv[2]);
    close(src);
    exit(1);
  }

  char buf[512];
  int n;
  while ((n = read(src, buf, sizeof(buf))) > 0) {
    if (write(dst, buf, n) != n) {
      fprintf(2, "cp: write error\n");
      close(src);
      close(dst);
      exit(1);
    }
  }
  // n = 0 -> read successfully
  if (n == -1) {
    fprintf(2, "cp: read error\n");
    close(src);
    close(dst);
    exit(1);
  }

  close(src);
  close(dst);
  exit(0);
}
