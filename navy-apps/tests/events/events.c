#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
  int fd = open("/dev/events", 0);
  if (fd < 0) {
    printf("open /dev/events failed\n");
    return 1;
  }

  printf("events test: click NEMU window, then press keys\n");

  while (1) {
    char buf[256];
    char *p = buf;
    ssize_t n;

    while ((n = read(fd, p, 1)) > 0) {
      if (*p == '\n') {
        p[1] = '\0';
        break;
      }
      p++;
    }

    if (p == buf) {
      continue;
    }

    printf("receive event: %s\n", buf);
  }

  return 0;
}
