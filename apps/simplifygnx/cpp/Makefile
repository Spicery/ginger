# EPREFIX should be bound to PREFIX by default. This appears to be a lie.
# As a consequence we override EPREFIX entirely. Probably incorrect but not
# that interesting.
prefix=/usr/local
exec_prefix=/usr/local
datarootdir=${prefix}/share
PACKAGE_TARNAME=appginger
INSTALL_BIN=${exec_prefix}/bin
INSTALL_LIB=${prefix}/share/appginger
INSTALL_DOC=${datarootdir}/doc/${PACKAGE_TARNAME}

#LICENSE_FILE="$(INSTALL_LIB)/LICENSE.txt"
#LICENSE_FILE="$(INSTALL_LIB)/LICENSE.txt"


CC=g++

CXXFLAGS=-DINSTALL_BIN='"$(INSTALL_BIN)"' -DINSTALL_LIB='"$(INSTALL_LIB)"' -g -I../hpp -I../../libgng/hpp -Wall -Wnon-virtual-dtor
CFLAGS=-Wmissing-prototypes 

OBJS= simplifygnx.o

all: simplifygnx

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
simplifygnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $^ ../../libgng/cpp/libgng.a
	
clean:
	rm -f *.o
	rm -f simplifygnx
	rm -f *.a

#	Nothing needs doing.
check:
