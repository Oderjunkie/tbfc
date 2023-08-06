ENTRY(_start)

SECTIONS {
  . = 0x08048070;
  .all : { __this = .; *(.text .rodata .rodata* .data .data* .bss) }
  /DISCARD/ : { *(.note.gnu.property .note.gnu.build-id .comment) }
}
