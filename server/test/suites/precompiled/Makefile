.PHONY: all clean

all: exe

clean:
	rm source.o exe

source.o: source.c
	clang -c -o source.o source.c

exe: source.o
	clang -o exe ../linkage-ld/issue-638.o source.o
