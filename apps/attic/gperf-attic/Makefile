all: item.o

CC=g++

CFLAGS=-m32 -I../headers -Wall -g -Wmissing-prototypes
CPPFLAGS=-m32 -I../headers -I../boost -Wall -g -Wmissing-prototypes


.SUFFIXES: .perf

item_perf.cc: item.perf
	gperf -D -t -T -L ANSI-C -e '#' item.perf > item_perf.cc
	
item.o: item_perf.cc
	$(CC) $(CPPFLAGS) -c -o item.o item_perf.cc
	
clean:
	rm -f item_perf.cc
	rm -f item.o
