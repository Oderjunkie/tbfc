#include <stdio.h>
#include <ctype.h>

int main(void) {
  unsigned char buf[16];
  unsigned long int offset = 0x000000;
  int read = 0;
  while ((read = fread(buf, 1, sizeof(buf), stdin)) > 0) {
    printf("%06lx: ", offset);
    for (int i = 0; i < 16; ++i) {
      if (i % 8 == 0) printf(" ");
      printf(i < read ? "%02x " : "   ", buf[i]);
    }
    printf(" # ");
    for (int i = 0; i < read; ++i)
      putchar(isprint(buf[i]) ? buf[i] : '.');
    printf("\n");
    offset += read;
  }
  return 0;
}
