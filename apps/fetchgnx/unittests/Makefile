
CC=g++

CXXFLAGS=-g -I../hpp -L/opt/local/lib -L../cpp -I/opt/local/include -I/usr/local/boost_1_43_0 -Wall -Wnon-virtual-dtor # -Weffc++

CFLAGS=-Wmissing-prototypes 

OBJS=testfetchgnx.o

all: testfetchgnx

check: testfetchgnx
	./testfetchgnx

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
testfetchgnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $^ -lcppunit
	
clean:
	rm -f *.o
	rm -f testfetchgnx
