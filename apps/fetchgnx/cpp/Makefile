
CC=g++

CXXFLAGS=-g -I../hpp -I/usr/local/boost_1_43_0 -Wall -Wnon-virtual-dtor # -Weffc++
CFLAGS=-Wmissing-prototypes 

OBJS=fetchgnx.o

all: fetchgnx

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
fetchgnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $^
	
clean:
	rm -f *.o
	rm -f fetchgnx

#	Nothing needs doing.
check:

