#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "user/user.h"

const int MAX_DEPTH = 30;
const int MAX_PATH = MAX_DEPTH * DIRSIZ;
const char OPENING = '1';
const char CLOSED = '0';

void
print_indent(int level, char *indent)
{
  for (int i = 0; i < level; ++i) {
    if (indent[i] == OPENING) {
      printf("│   ");
    } else {
      printf("    ");
    }
  }
}

int
is_num(char *str)
{
  for (int i = 0; str[i] != '\0'; ++i) {
    if (str[i] < '0' || str[i] > '9') {
      return 0;
    }
  }
  return 1;
}

void
tree(char *path, int level, int depth, int only_dir, char *indent)
{
  if (level >= depth) {
    return;
  }

  int fd;
  struct stat st;
  struct dirent di;
  char child[MAX_PATH];
  char name[DIRSIZ + 1];

  fd = open(path, 0);
  if (fd < 0) {
    printf("tree: cannot open %s\n", path);
    return;
  }

  if (fstat(fd, &st) < 0) {
    printf("tree: cannot stat %s\n", path);
    close(fd);
    return;
  }

  int n = read(fd, &di, sizeof(di));
  while (n == sizeof(di)) {
    if (di.inum == 0) {
      n = read(fd, &di, sizeof(di));
      continue;
    }

    memmove(name, di.name, DIRSIZ);
    name[DIRSIZ] = '\0';

    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
      n = read(fd, &di, sizeof(di));
      continue;
    }

    int len = strlen(path);
    if (len + 1 + strlen(name) + 1 > MAX_PATH) {
      printf("tree: path too long\n");
      exit(1);
    }

    memmove(child, path, len);
    child[len] = '/';
    memmove(child + len + 1, name, strlen(name) + 1);

    struct stat st2;
    stat(child, &st2);

    n = read(fd, &di, sizeof(di));
    if (!only_dir || st2.type == T_DIR) {
      print_indent(level, indent);
      if (n != sizeof(di)) {
        printf("└───");
        indent[level] = CLOSED;
      } else {
        printf("├───");
        indent[level] = OPENING;
      }
      printf("%s\n", name);
    }

    if (st2.type == T_DIR) {

      tree(child, level + 1, depth, only_dir, indent);
    }
  }

  close(fd);
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int depth = MAX_DEPTH;
  int only_dir = 0;
  int path_set = 0;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-d") == 0) {
      only_dir = 1;
      continue;
    }

    if (strcmp(argv[i], "-L") == 0) {
      if (i + 1 == argc) {
        printf("usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }

      if (!is_num(argv[i + 1])) {
        printf("usage: tree [path] [-L depth] [-d]\n");
        exit(1);
      }
      depth = atoi(argv[i + 1]);

      ++i;
      continue;
    }

    if (argv[i][0] == '-') {
      printf("usage: tree [path] [-L depth] [-d]\n");
      exit(1);
    }

    if (!path_set) {
      path_set = 1;
      path = argv[i];
      continue;
    }

    printf("usage: tree [path] [-L depth] [-d]\n");
    exit(1);
  }

  struct stat st;

  if (stat(path, &st) < 0 || st.type != T_DIR) {
    printf("tree: %s is not a path\n", path);
    exit(1);
  }

  printf("%s\n", path);
  char indent[MAX_DEPTH];
  tree(path, 0, depth, only_dir, indent);

  return 0;
}