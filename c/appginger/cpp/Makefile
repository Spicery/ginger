CC=g++

CPPFLAGS=-I../hpp -I../../automatic -I/usr/local/boost_1_43_0 -Wall -g -Wmissing-prototypes

OBJS=mishap.o term.o read_xml.o \
	functor.o plant.o cage.o machine.o vmi.o gc.o \
	rcep.o arity.o lift.o ident.o dict.o sys.o \
	destination.o initialize.o cgi.o heap.o \
	scanfunc.o \
        instruction_set.o \
	machine1.o \
	instruction_set1.o \
	machine2.o \
	instruction_set2.o \
	machine3.o \
	instruction_set3.o \
	appginger.o

appginger: $(OBJS)
	$(CC) $(CPPFLAGS) -o appginger $(OBJS)
	
clean:
	rm -f *.o
	rm -f appginger
	
libappginger.a: $(OBJS)
	ar -rcs libappginger.a $(OBJS)
