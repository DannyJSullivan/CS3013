all: bathroomsim test1 test2 test3

bathroomsim: bathroom.o bathroomsim.o bathroom.h
	gcc bathroom.o bathroomsim.o -o bathroomsim -Wall -pthread -lm

bathroomsim.o: bathroomsim.c bathroom.h
	gcc -g -c bathroomsim.c -Wall -pthread

bathroom.o: bathroom.c bathroom.h
	gcc -g -c bathroom.c -pthread

test1: test1.c
	gcc test1.c -o test1

test2: test2.c
	gcc test2.c -o test2

test3: test3.c
	gcc test3.c -o test3

clean:
	rm bathroomsim
	rm *.o
	rm test1 test2 test3
