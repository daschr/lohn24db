CC=clang
FLAGS=-DDEBUG -lulfius -lpq

all:	*.c
	$(CC) $(FLAGS) -o lohn24pg  $^
