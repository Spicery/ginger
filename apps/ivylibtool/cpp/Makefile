prefix=/usr/local
exec_prefix=/usr/local
datarootdir=${prefix}/share
PACKAGE_TARNAME=ginger
INSTALL_BIN=${exec_prefix}/bin
INSTALL_TOOL=${exec_prefix}/libexec/ginger
INSTALL_LIB=${prefix}/share/ginger
INSTALL_DOC=${datarootdir}/doc/${PACKAGE_TARNAME}

CC=g++
CXX=g++


INSTALLDIRS=-DINSTALL_BIN='"$(INSTALL_BIN)"' -DINSTALL_LIB='"$(INSTALL_LIB)"' -DINSTALL_TOOL='"$(INSTALL_TOOL)"'

# -Weffc++
CXXFLAGS=$(INSTALLDIRS) -g -std=c++0x -I../hpp -I../../libgng/hpp -Wall -Wnon-virtual-dtor
CFLAGS=-Wmissing-prototypes 

OBJS=ivylibtool.o

all: ivylibtool

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
ivylibtool: $(OBJS)
	$(CC) $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $(OBJS) ../../libgng/cpp/libgng.a

clean:
	rm -f *.o
	rm -f ivylibtool

#	Nothing needs doing.
check:
