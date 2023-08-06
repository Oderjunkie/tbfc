#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    fprintf(stderr, "./insertconstants [file]\n");
    fprintf(stderr, "inserts a couple constants required by tbfc into unused parts of ELF\n");
    return 1;
  }
  FILE *f;
  if (!(f = fopen(argv[argc - 1], "r+"))) {
    fprintf(stderr, "sectionless: %s\n", strerror(errno));
    return 1;
  }
  fseek(f, 0x28, SEEK_SET);
  assert(fwrite(&(char[]){ 0xFC, 0xAC, 0xFD, 0xAC, 0x38, 0x1E, 0x0F, 0x84, 0xFE, 0x06, 0xFE, 0x0E }, 1, 12, f) == 12);
  fseek(f, 0x04, SEEK_SET);
  assert(fwrite(&(char[]){ 0xB2, 0x01, 0x48, 0xBE, 0x31, 0xC0, 0xB0, 0x01, 0x89, 0xC7, 0x0F, 0x05 }, 1, 12, f) == 12);
  fseek(f, 0x50, SEEK_SET);
  assert(fwrite(&(char[]){ 0x31, 0xC0, 0x89, 0xC7, 0x0F, 0x05, 0xE9 }, 1, 7, f) == 7);
  fclose(f);
  return 0;
}
