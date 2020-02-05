all: dir

dir: dir.o
	gcc dir.o -o dir -Wall -pedantic -O2

dir.o: dir.c
	gcc -c dir.c -o dir.o -Wall -pedantic -O2

clean:
	rm *.o

run:
	./dir
