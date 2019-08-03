CC=clang
FLAGS=-DDEBUG -lcrypto -lulfius -lpq

all:	*.c
	$(CC) $(FLAGS) -o lohn24pg  $^
