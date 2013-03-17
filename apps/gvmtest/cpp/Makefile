prefix=/usr/local
exec_prefix=/usr/local
datarootdir=${prefix}/share
PACKAGE_TARNAME=ginger
INSTALL_BIN=${exec_prefix}/bin
INSTALL_TOOL=${exec_prefix}/libexec/ginger
INSTALL_LIB=${prefix}/share/ginger
INSTALL_DOC=${datarootdir}/doc/${PACKAGE_TARNAME}

#LICENSE_FILE="$(INSTALL_LIB)/COPYING"
#LICENSE_FILE="$(INSTALL_LIB)/COPYING"

CC=g++

INSTALLDIRS=-DINSTALL_BIN='"$(INSTALL_BIN)"' -DINSTALL_LIB='"$(INSTALL_LIB)"' -DINSTALL_TOOL='"$(INSTALL_TOOL)"'

CFLAGS=-Wall -Wmissing-prototypes -Wnon-virtual-dtor
CXXFLAGS=$(CFLAGS) -DRUDECGI $(INSTALLDIRS) -g -I../hpp -I../../appginger/hpp -I../../libgng/hpp -I../../automatic/machine -I../../automatic/simple

OBJS=gvmtest.o gvm.o

all: gvmtest

################################################################################
# 	Main executables
################################################################################

# $@ is make shorthand for the target.
# $@ is make shorthand for the dependencies
gvmtest: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o $@ $(OBJS) ../../appginger/cpp/libappginger.a ../../libgng/cpp/libgng.a /usr/local/lib/librudecgi.a

clean:
	rm -f *.o
	rm -f gvmtest
	rm -f *.a

#	Nothing needs doing.
check:
