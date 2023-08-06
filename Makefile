.PHONY : clean
all : main

CFLAGS := -fno-asynchronous-unwind-tables -static
override CFLAGS += -Os -nostdlib
LDFLAGS := -T linker.ld
sources := $(wildcard src/*.c)
dependancies := $(subst src,build,$(sources:.c=.d))
objects := $(subst src,build,$(sources:.c=.o))

include $(dependancies)

build/%.d : src/%.c
	$(CC) $(CFLAGS) -MM $^ > $@
build/%.o : src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^
tools/% : tools/%.c
	$(CC) -o $@ $^
main : $(objects) | tools/sectionless tools/insertconstants
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^
	strip -s $@
	objdump -d $@
	./tools/sectionless $@
	./tools/insertconstants $@
clean :
	rm build/* src/*.gch
