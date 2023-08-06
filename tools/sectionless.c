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
    fprintf(stderr, "./sectionless [file]\n");
    fprintf(stderr, "turns a standard ELF file into an identically-behaving\n");
    fprintf(stderr, "ELF file that doesn't contain any sections.\n");
    return 1;
  }
  FILE *f;
  if (!(f = fopen(argv[argc - 1], "r+"))) {
    fprintf(stderr, "sectionless: %s\n", strerror(errno));
    return 1;
  }

  assert(fgetc(f) == 0x7f);
  assert(fgetc(f) == 0x45);
  assert(fgetc(f) == 0x4c);
  assert(fgetc(f) == 0x46);
  assert(fgetc(f) == 0x02); /* 64-bit */
  assert(fgetc(f) == 0x01); /* little-endian */
  assert(fgetc(f) == 0x01);
  assert(fgetc(f) == 0x00); /* sysv */
  fseek(f, 0x10, SEEK_SET);
  assert(fgetc(f) == 0x02); /* executable file */
  assert(fgetc(f) == 0x00); /* |               */
  assert(fgetc(f) == 0x3e); /* instruction set architecture */
  assert(fgetc(f) == 0x00); /* |                            */
  assert(fgetc(f) == 0x01);
  assert(fgetc(f) == 0x00);
  assert(fgetc(f) == 0x00);
  assert(fgetc(f) == 0x00);
  uintptr_t entry; assert(fread(&entry, sizeof(entry), 1, f) == 1);
  uintptr_t phoff; assert(fread(&phoff, sizeof(phoff), 1, f) == 1);
  uintptr_t shoff; assert(fread(&shoff, sizeof(shoff), 1, f) == 1);
  fseek(f, 0x3a, SEEK_SET);
  uint16_t shentsize, shnum, shstrndx;
  assert(fread(&shentsize, sizeof(shentsize), 1, f) == 1);
  assert(fread(&shnum, sizeof(shnum), 1, f) == 1);
  assert(fread(&shstrndx, sizeof(shstrndx), 1, f) == 1);
  assert(shstrndx < shnum);
  assert(phoff == 0x40);

  fseek(f, shoff + shentsize * shstrndx + 0x18, SEEK_SET);
  uintptr_t symtable;
  assert(fread(&symtable, sizeof(symtable), 1, f) == 1);

  uint16_t allidx;
  for (allidx = 0; allidx < shnum; ++allidx) {
    fseek(f, shoff + shentsize * allidx, SEEK_SET);
    uint32_t sh_name;
    assert(fread(&sh_name, sizeof(sh_name), 1, f) == 1);
    fseek(f, symtable + sh_name, SEEK_SET);
    char buffer[5] = { 0 };
    assert(fread(&buffer, 1, sizeof(buffer), f) == sizeof(buffer));
    if (!memcmp(buffer, ".all", sizeof(buffer))) {
      fprintf(stderr, "sectionless: .all section is number 0x%04" PRIX16 "\n", allidx);
      break;
    }
  }

  fseek(f, 0x28, SEEK_SET);
  assert(fwrite(&(uintptr_t){ 0 }, sizeof(uintptr_t), 1, f) == 1); /* set the offset to the table of sections to 0 */
  fseek(f, 0x38, SEEK_SET);
  assert(fwrite(&(uint16_t){ 1 }, sizeof(uint16_t), 1, f) == 1); /* set the number of program headers to 1 */
  fseek(f, 0x3a, SEEK_SET);
  assert(fwrite(&(uint16_t){ 0 }, sizeof(uint16_t), 1, f) == 1); /* set the size of each section to 0 */
  assert(fwrite(&(uint16_t){ 0 }, sizeof(uint16_t), 1, f) == 1); /* set the number of sections to 0 */
  assert(fwrite(&(uint16_t){ 0 }, sizeof(uint16_t), 1, f) == 1); /* set the index of the .shstr section to 0 */
  
  fseek(f, shoff + shentsize * allidx + 0x10, SEEK_SET);
  uintptr_t sh_addr; assert(fread(&sh_addr, sizeof(sh_addr), 1, f) == 1);
  uintptr_t sh_offset; assert(fread(&sh_offset, sizeof(sh_offset), 1, f) == 1);
  uintptr_t sh_size; assert(fread(&sh_size, sizeof(sh_size), 1, f) == 1);
  
  char *stream = malloc(sh_size);
  fseek(f, sh_offset, SEEK_SET);
  assert(fread(stream, 1, sh_size, f) == sh_size);
  fseek(f, 0x70, SEEK_SET);
  assert(fwrite(stream, 1, sh_size, f) == sh_size);
  free(stream);
  sh_addr -= 0x70;
  sh_size += 0x70;
  sh_offset = 0x00;
  phoff = 0x38;
  uintptr_t p_filesz = sh_size;
  for (;;) {
    fseek(f, -1, SEEK_END);
    fseek(f, p_filesz - 1, SEEK_SET);
    if (fgetc(f) == 0x00)
      --p_filesz;
    else
      break;
  }

  fseek(f, 0x38, SEEK_SET);
  assert(fwrite(&(uint32_t){ 1 }, sizeof(uint32_t), 1, f) == 1); /* p_type = PT_LOAD */
  assert(fwrite(&(uint32_t){ 7 }, sizeof(uint32_t), 1, f) == 1); /* p_flags = PF_X | PF_W | PF_R */
  assert(fwrite(&sh_offset, sizeof(sh_offset), 1, f) == 1); /* p_offset = sh_offset */
  assert(fwrite(&sh_addr, sizeof(sh_addr), 1, f) == 1); /* p_vaddr = sh_addr */
  assert(fwrite(&sh_addr, sizeof(sh_addr), 1, f) == 1); /* p_paddr = sh_addr */
  assert(fwrite(&p_filesz, sizeof(p_filesz), 1, f) == 1); /* p_filesz = p_filesz */
  assert(fwrite(&sh_size, sizeof(sh_size), 1, f) == 1); /* p_memsz = sh_size */
  fseek(f, 0x68, SEEK_SET);
  assert(fwrite(&(uintptr_t){ 1 }, sizeof(uintptr_t), 1, f) == 1); /* p_align = 1 */
  fseek(f, 0x18, SEEK_SET);
  assert(fwrite(&entry, sizeof(entry), 1, f) == 1); /* entry */
  assert(fwrite(&phoff, sizeof(phoff), 1, f) == 1);

  fseek(f, 0, SEEK_END);
  fprintf(stderr, "sectionless: file size decreased from %lu to %lu\n", (unsigned long int) ftell(f), (unsigned long int) p_filesz);
  assert(ftruncate(fileno(f), p_filesz) == 0);

  fclose(f);
  return 0;
}
