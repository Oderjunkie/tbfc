#include <stddef.h>
typedef signed long long int ssize_t;

#ifdef __GNUC__
  #define unreachable __builtin_unreachable()
#else
  #define unreachable do {} while (0)
#endif
#define exit(value) do { _exit0(value, _exit4_##value); unreachable; } while (0)
#define _exit0(value, ...) _exit1(value, __VA_ARGS__)
#define _exit1(value, ...) _exit2(__VA_ARGS__, _exit3(value), ~)
#define _exit2(a, b, ...) b
#define _exit3(value) asm("xor %eax, %eax\nmov $" #value ", %di\n mov $60, %al\n syscall")
#define _exit4_0 ~,asm("xor %eax, %eax\nxor %edi, %edi\n mov $60, %al\n syscall")
#define _exit4_1 ~,asm("xor %eax, %eax\nxor %edi, %edi\n inc %rdi\n mov $60, %al\n syscall")

static inline ssize_t write(int fd, const void *buf, size_t length) {
  register ssize_t out;
  asm volatile ("syscall" : "=a" (out) : "a" (1), "D" (fd), "S" (buf), "d" (length) : "rcx", "r11");
  return out;
}

static inline ssize_t read(int fd, void *buf, size_t length) {
  register ssize_t out;
  asm volatile ("syscall" : "=a" (out) : "a" (0), "D" (fd), "S" (buf), "d" (length) : "rcx", "r11");
  return out;
}

static inline void *brk(const void *data) {
  register void *out;
  asm volatile ("syscall" : "=a" (out) : "a" (12), "D" (data) : "rcx", "r11");
  return out;
}

static inline void *memcpy(void *dst, const void *src, size_t len) {
  char *cdst = dst;
  const char *csrc = src;
  while (len --> 0)
    *cdst++ = *csrc++;
  return dst;
}
