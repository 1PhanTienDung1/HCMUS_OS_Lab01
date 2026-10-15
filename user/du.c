#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/param.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static int all = 0;     // -a
static int summary = 0; // -s

//
struct walk {
  char *path;
  int root;
  struct stat info;
  struct dirent *entries;
  char child[MAXPATH];
  char name[DIRSIZ + 1];
  uint64 total;
  int fd;
  int offset;
  int n;
  int i;
};

static void
fail(const char *message, const char *path)
{
  fprintf(2, "du: %s: %s\n", message, path);
  exit(1);
}

static struct walk *__attribute__((noinline))
load_entry(char *path, int root)
{
  // malloc: xin vung nho du chua size cua struct walk
  struct walk *w = malloc(sizeof(*w));
  if (w == 0)
    fail("out of memory", path);

  w->fd = open(path, O_RDONLY);
  if (w->fd == -1)
    fail("cannot open", path);
  if (fstat(w->fd, &w->info) == -1)
    fail("cannot stat", path);

  w->path = path;
  w->root = root;
  w->entries = 0;
  w->i = 0;
  w->total = 0;
  if (w->info.type == T_DIR) {
    w->entries = malloc(w->info.size ? w->info.size : 1);
    if (w->entries == 0)
      fail("out of memory", path);

    w->offset = 0;
    while (w->offset < w->info.size) {
      w->n =
        read(w->fd, (char *)w->entries + w->offset, w->info.size - w->offset);
      if (w->n <= 0)
        fail("cannot read directory", path);
      w->offset += w->n;
    }
    close(w->fd);
    if (w->info.size % sizeof(struct dirent) != 0)
      fail("invalid directory", path);

  } else {
    close(w->fd);
    if (w->info.type == T_FILE)
      w->total = w->info.size;
  }
  return w;
}

static int __attribute__((noinline))
next_child(struct walk *w)
{
  if (w->info.type != T_DIR)
    return 0;

  while (w->i < w->info.size / sizeof(struct dirent)) {
    int i = w->i++;
    if (w->entries[i].inum == 0)
      continue;

    memmove(w->name, w->entries[i].name, DIRSIZ);
    w->name[DIRSIZ] = '\0';
    if (strcmp(w->name, ".") == 0 || strcmp(w->name, "..") == 0)
      continue;

    w->offset = strlen(w->path);
    if (w->offset > 0 && w->path[w->offset - 1] == '/')
      w->offset--;
    if (w->offset + 1 + strlen(w->name) + 1 > MAXPATH)
      fail("path too long", w->path);
    memmove(w->child, w->path, w->offset);
    w->child[w->offset] = '/';
    strcpy(w->child + w->offset + 1, w->name);
    return 1;
  }
  return 0;
}

static uint64
du(char *path, int root)
{
  struct walk *w = load_entry(path, root);

  while (next_child(w)) {
    uint64 child_size = du(w->child, 0);
    w->total += child_size;
  }

  if (w->root || (!summary && (all || w->info.type == T_DIR)))
    printf("%lu\t%s\n", w->total, w->path);

  uint64 total = w->total;
  if (w->entries != 0)
    free(w->entries);
  free(w);
  return total;
}

int
main(int argc, char *argv[])
{
  char *path = ".";
  int ps = 0;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-a") == 0) {
      all = 1;
    } else if (strcmp(argv[i], "-s") == 0) {
      summary = 1;
    } else if (argv[i][0] == '-' || ps) {
      fprintf(2, "usage: du [path] [-a] [-s]\n");
      exit(1);
    } else {
      path = argv[i];
      ps = 1;
    }
  }
  // DFS
  du(path, 1);
  exit(0);
}
