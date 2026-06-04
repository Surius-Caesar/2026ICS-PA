#include <stdio.h>

int main() {
  FILE *fp = fopen("/dev/events", "r");
  if (fp == NULL) {
    printf("fopen /dev/events failed\n");
    return 1;
  }

  printf("events test: reading /dev/events (click NEMU window, then press keys)\n");

  while (1) {
    char buf[256];
    char *p = buf;
    int ch;

    while ((ch = fgetc(fp)) != EOF) {
      *p++ = (char)ch;
      if (ch == '\n') {
        *p = '\0';
        break;
      }
    }

    if (p == buf) {
      continue;
    }

    printf("receive event: %s", buf);
  }

  return 0;
}
