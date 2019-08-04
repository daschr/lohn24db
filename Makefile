CC=clang
FLAGS=-DDEBUG -lcrypt -lulfius -lpq
#FLAGS=-lcrypto -lulfius -lpq

all:	*.c
	$(CC) $(FLAGS) -o lohn24pg  $^
