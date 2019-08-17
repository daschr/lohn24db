CC=clang
LIBS=-lcrypt -lulfius -ljansson -lpq -lyder
DBG=-DDEBUG 

all:	*.c
	$(CC) -fPIC -shared $(DBG) $(LIBS) -o lohn24db.so  $^
ins: lohn24db.so
	sudo cp lohn24db.so /usr/lib/glewlwyd/user/libusermodlohn24db.so
run: 
	sudo glewlwyd -c/usr/etc/glewlwyd/glewlwyd.conf -lDEBUG
clean: lohn24db.so
	rm lohn24db.so
