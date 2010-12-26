
CC=g++

CXXFLAGS=-g -I../hpp -L/opt/local/lib -L../cpp -I/opt/local/include -I/usr/local/boost_1_43_0 -Wall -Wnon-virtual-dtor # -Weffc++

CFLAGS=-Wmissing-prototypes 

OBJS=testlibgng.o

all: testlibgng

check: testlibgng
	./testlibgng

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
testlibgng: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -L../cpp -o $@ $^ -lcppunit -lgng
	
clean:
	rm -f *.o
	rm -f testlibgng
