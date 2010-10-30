
CC=g++

CXXFLAGS=-g -I../hpp -L/opt/local/lib -L../cpp -I/opt/local/include -I../../automatic/machine -I../../automatic/sys -I../../automatic/simple -I/usr/local/boost_1_43_0 -Wall -Wnon-virtual-dtor # -Weffc++
CFLAGS=-Wmissing-prototypes 

OBJS=example.o testrunner.o gctest.o gcstats.o archtest.o

all: testrunner

check: testrunner
	./testrunner

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
testrunner: $(OBJS) ../cpp/libappginger.a
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $^ -lcppunit -lappginger
	
clean:
	rm -f *.o
	rm -f testrunner
	rm -f *.a
