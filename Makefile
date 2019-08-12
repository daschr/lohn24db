CC=clang
FLAGS=-DDEBUG -lcrypt -lulfius -lpq
#FLAGS=-lcrypt -lulfius -lpq

all:	*.c
	$(CC) $(FLAGS) -o lohn24db  $^

clean:
	rm lohn24db
