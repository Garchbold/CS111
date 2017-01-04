

all: lab0

lab0: lab0.o
	gcc -Werror -g lab0.o -o lab0

lab0.o:
	gcc -Werror -g -c lab0.c

clean:
	rm *.o lab0

check:
	touch input.txt output.txt
	echo hello > input.txt
	./lab0 --input=input.txt --output=output.txt
	cat input.txt | diff - output.txt 
	./lab0 --input=  2> /dev/null || ([ $$? -eq 1 ] && echo "Return code for input check OK")
	./lab0 --output=  2> /dev/null || ([ $$? -eq 2 ] && echo "Return code for output check OK")
	./lab0 --segfault --catch > /dev/null  2> /dev/null || ([ $$? -eq 3 ] && echo "Return code for catch OK")

dist:
	tar -zcvf lab0-604407413.tar.gz lab0 lab0.c Makefile README breakpoint.png segfault.png
