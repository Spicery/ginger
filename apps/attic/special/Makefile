# -m32 forces 32-bit void* fields
CC=g++

CPPFLAGS=-m32 -I../headers -I../boost -I../code -Wall -g -Wmissing-prototypes


special.o: *.c *.auto
	$(CC) $(CPPFLAGS) -c -o special.o special.c
	
clean:
	rm *.o
	
