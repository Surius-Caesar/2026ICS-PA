#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main() {
  int fd = open("/dev/events", O_RDONLY);
  if (fd < 0) {
    printf("open /dev/events failed\n");
    return 1;
  }

  printf("events test: click NEMU window, then press keys\n");

  char buf[256];
  int n = 0;

  while (1) {
    char c;
    ssize_t r = read(fd, &c, 1);
    if (r < 0) {
      continue;
    }
    if (r == 0) {
      printf("read returned 0\n");
      continue;
    }
    if (c == '\0') {
      printf("read null byte (events_read bug?)\n");
      continue;
    }
    buf[n++] = c;
    if (c == '\n') {
      buf[n] = '\0';
      printf("receive event: %s", buf);
      n = 0;
    }
    if (n >= (int)sizeof(buf) - 1) {
      n = 0;
    }
  }

  return 0;
}
