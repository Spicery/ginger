CC=g++

CXXFLAGS=-I../hpp -I../../libgng/hpp -I/usr/local/boost_1_43_0 -Wall -g
CFLAGS=-Wmissing-prototypes 

OBJS=gson2gnx.o  item_factory.o item.o toktype.o

gson2gnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o gson2gnx $(OBJS) ../../libgng/cpp/libgng.a
	
clean:
	rm -f *.o
	rm -f gson2gnx
