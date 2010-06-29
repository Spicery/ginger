CC=g++

CPPFLAGS=-I../hpp -I../../automatic -I../../appginger/hpp -I/usr/local/boost_1_43_0 -Wall -g -Wmissing-prototypes 

OBJS=common2gnx.o read_expr.o item.o item_map.o item_factory.o functor.o node.o mishap.o role.o

common2gnx: $(OBJS)
	$(CC) $(CPPFLAGS) -o common2gnx $(OBJS)
	
clean:
	rm -f *.o
	rm -f common2gnx
