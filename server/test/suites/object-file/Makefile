.PHONY: all clean

all: exe

clean:
	rm exe lib.a lib1.o lib2.o

lib1.o: lib1.c
	clang -c -o lib1.o lib1.c

lib2.o: lib2.c
	clang -c -o lib2.o lib2.c

lib.a: lib1.o lib2.o
	ar crs -o lib.a lib1.o lib2.o

exe: source1.c source2.c lib.a
	clang -o exe source1.c source2.c lib.a
