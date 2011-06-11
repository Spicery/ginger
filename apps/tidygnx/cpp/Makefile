CC=g++

CXXFLAGS=-I../hpp -I../../libgng/hpp -I/usr/local/boost_1_43_0 -Wall -g
CFLAGS=-Wmissing-prototypes 

OBJS=tidygnx.o

gson2gnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o tidygnx $(OBJS) ../../libgng/cpp/libgng.a
	
clean:
	rm -f *.o
	rm -f tidygnx
