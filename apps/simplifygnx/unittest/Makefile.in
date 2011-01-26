
CC=g++

CXXFLAGS=-g -I../hpp -I../../libgng/hpp -Wall -Wnon-virtual-dtor
CFLAGS=-Wmissing-prototypes 
LDFLAGS=-ldl -lboost_iostreams
OBJS=testsimplifygnx.o

all: testsimplifygnx

check: testsimplifygnx
	./testsimplifygnx 2> results.xml 

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
testsimplifygnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $^ ../../libgng/cpp/libgng.a  -lcppunit $(LDFLAGS)
	
clean:
	rm -f *.o
	rm -f testsimplifygnx
	rm -f *.a
