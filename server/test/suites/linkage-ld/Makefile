.PHONY: all clean

all: issue-638.so

clean:
	rm issue-638.o issue-638.so

issue-638.o: issue-638.c
	clang -c -o issue-638.o issue-638.c

issue-638.so: issue-638.o
	ld --shared -o issue-638.so issue-638.o
