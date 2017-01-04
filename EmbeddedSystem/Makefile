LIBS = -L/usr/lib -lssl -lcrypto
OBJS = tls_header.o

all: tls_client 

tls_client: tls_client.c tls_header.o
	gcc -lpthread -w -o tls_client tls_client.c $(OBJS) $(LIBS)
	
tls_header.o: tls_header.c
	gcc -c -o tls_header.o tls_header.c $(LIBS)

clean:
	rm -f set_rate start_rate tls_client tls_header.o

clobber: 
	make clean
	rm -f *.txt *~

dist:
	tar -zcvf lab4-604407413.tar.gz tls_client.c Makefile README Lab4-B-1.jpg Lab4-B-2.jpg Lab4-B-3.jpg Lab4-B-4.jpg Lab4-D-1.jpg Lab4-D-2.jpg Lab4-D-3.jpg Lab4-D-4.jpg Lab4-E-1.jpg Lab4-E-2.jpg
