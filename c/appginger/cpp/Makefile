

CC=g++

CPPFLAGS=-g -I../hpp -I../../automatic/machine -I../../automatic/sys -I/usr/local/boost_1_43_0 -Wall -Wmissing-prototypes

OBJS=mishap.o term.o read_xml.o \
	functor.o plant.o cage.o vmi.o gc.o \
	rcep.o arity.o lift.o ident.o dict.o sys.o \
	destination.o heap.o \
	key.o objlayout.o \
	heapcrawl.o cagecrawl.o callstackcrawl.o fnobjcrawl.o \
	garbagecollect.o \
	scanfunc.o scandict.o \
	machine.o \
	instruction_set.o \
	machine1.o \
	instruction_set1.o \
	machine2.o \
	instruction_set2.o \
	machine3.o \
	instruction_set3.o \
	appginger.o

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
appginger: $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $+
	
clean:
	rm -f *.o
	rm -f appginger

#	Nothing needs doing.
check:
	
libappginger.a: $(OBJS)
	ar -rcs $@ $+
