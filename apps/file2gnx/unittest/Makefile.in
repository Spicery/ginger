
CC=g++

CXXFLAGS=-g -I../hpp -L/opt/local/lib -L../cpp -I/opt/local/include -I/usr/local/boost_1_43_0 -Wall -Wnon-virtual-dtor # -Weffc++

CFLAGS=-Wmissing-prototypes 

OBJS=testfile2gnx.o

all: testfile2gnx

check: testfile2gnx
	./testfile2gnx

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
testfile2gnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $^ -lcppunit
	
clean:
	rm -f *.o
	rm -f testfile2gnx
