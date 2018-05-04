#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>

int
access(const char *path, int mode)
{
  struct stat buf;
  if (stat(path, &buf) != -1)
  {
    return 0;
  }
  else {
    errno = ENOENT;
  }

  return -1;
}
