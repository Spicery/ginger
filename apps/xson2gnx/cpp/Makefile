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

CC=g++

CXXFLAGS=-DINSTALL_BIN='"$(INSTALL_BIN)"' -DINSTALL_LIB='"$(INSTALL_LIB)"' -I../hpp -I../../libgng/hpp -I/usr/local/boost_1_43_0 -Wall -g
CFLAGS=-Wmissing-prototypes 

OBJS=xson2gnx.o item_factory.o item.o toktype.o xsonparser.o

xson2gnx: $(OBJS)
	g++ $(CXXFLAGS) $(BOOST_CPPFLAGS) -o xson2gnx $(OBJS) ../../libgng/cpp/libgng.a
	
clean:
	rm -f *.o
	rm -f xson2gnx
