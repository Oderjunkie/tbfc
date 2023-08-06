#include "hacks.h"
#define TAPE_SIZE 1024
#include <stdint.h>

extern void __this;
static const void *this = ((char *) &__this) - 0x70;

static const unsigned char *INIT_INST = (unsigned char *) &__this - 0x6c;
static const unsigned char *INC_INST = (unsigned char *) &__this - 0x40;
static const unsigned char *DEC_INST = (unsigned char *) &__this - 0x3E;
static const unsigned char *RIGHT_INST = (unsigned char *) &__this - 0x48;
static const unsigned char *LEFT_INST = (unsigned char *) &__this - 0x46;
static const unsigned char *OUT_INST = (unsigned char *) &__this - 0x68;
static const unsigned char *IN_INST = (unsigned char *) &__this - 0x20;
static const unsigned char *BEGIN_INST = (unsigned char *) &__this - 0x44;
static const unsigned char *END_INST = (unsigned char *) &__this - 0x1a;

static size_t len = 0x00;
static void *oldbrk;

void *sbrk(size_t shift) {
  register size_t oldlen = len;
  len += shift;
  brk((char *) oldbrk + len);
  return (void *) ((char *) oldbrk + oldlen);
}

static void write_to_elf(const void *ptr, size_t size) {
  memcpy(sbrk(size), ptr, size);
}

static void compile(void) {
  char c;
  while (read(0, &c, 1) == 1) {
    register const void *inst; register size_t instsize;
    inst = (void *) 0;
    instsize = 0;
    switch (c) {
      case '+': inst = INC_INST; instsize = 2; break;
      case '-': inst = DEC_INST; instsize = 2; break;
      case '>': inst = RIGHT_INST; instsize = 2; break;
      case '<': inst = LEFT_INST; instsize = 2; break;
      case '.': inst = OUT_INST; instsize = 8; break;
      case ',': inst = IN_INST; instsize = 6; break;
      case '[': {
        write_to_elf(BEGIN_INST, 8);
        register uint32_t *off = (void *) ((char *) oldbrk + len - 4);
        register size_t oldlen = len;
        compile();
        *off = (int32_t) (len - oldlen);
        off = (void *) ((char *) oldbrk + len - 4);
        *off = (int32_t) (oldlen - len - 8);
      } continue;
      case ']': {
        write_to_elf(END_INST, 5);
        return;
      }
    }
    write_to_elf(inst, instsize);
  }
}

void EXIT_INST(void);

void _start(void) {
  oldbrk = brk((void *) 0);
  write_to_elf(this, 0x70);
  write_to_elf(INIT_INST, 12);
  compile();
  write_to_elf(&EXIT_INST, 8);
  uintptr_t *size = (uintptr_t *) ((char *) oldbrk + 0x58);
  size[0] = len;
  size[1] = len + TAPE_SIZE;
  register uintptr_t *entrypoint = (uintptr_t *) ((char *) oldbrk + 0x18);
  *entrypoint = size[-2] + 0x70;
  register uintptr_t *memory_tape = (uintptr_t *) ((char *) oldbrk + 0x74);
  *memory_tape = *entrypoint + len;
  write(1, oldbrk, len);
  EXIT_INST();
}

void EXIT_INST(void) { exit(0); }
