all: process

process: main.o process.o
	gcc main.o process.o -o process -Wall -pedantic -O2 -g

main.o: main.c
	gcc -c main.c -o main.o -Wall -pedantic -O2 -g

process.o: process.c
	gcc -c process.c -o process.o -Wall -pedantic -O2 -g

clean:
	rm *.o

run:
	./process
