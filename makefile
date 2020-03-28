
FLAGS = -Wall -Wextra -Werror -pedantic -pedantic-errors -O2 -g

all: process

process: main.o process.o network.o
	gcc main.o process.o network.o -o process $(FLAGS)

main.o: main.c
	gcc -c main.c -o main.o $(FLAGS)

process.o: process.c
	gcc -c process.c -o process.o $(FLAGS)

network.o: network.c
	gcc -c network.c -o network.o $(FLAGS)

clean:
	rm *.o

run:
	sudo ./process
