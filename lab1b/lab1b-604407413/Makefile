
all: client.o server.o
	gcc -lmcrypt -lpthread client.o -o client
	gcc -lmcrypt -lpthread server.o -o server

client.o: client.c
	gcc -lmcrypt -lpthread -c client.c

server.o: server.c
	gcc -lmcrypt -lpthread -c server.c

clean:
	rm *.o

dist:
	tar -zcvf lab1b-604407413.tar.gz client.c server.c Makefile my.key README
