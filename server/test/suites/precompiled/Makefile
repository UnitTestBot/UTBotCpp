.PHONY: all clean

all: exe

clean:
	rm source2.o exe

source2.o: source2.c
	clang -c -o source2.o source2.c

exe: source2.o
	clang -o exe ../object-file/source1.o source2.o
