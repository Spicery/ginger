CC=g++

CPPFLAGS=-Wall -g -Wmissing-prototypes

INCLUDES=-I../headers

TARGET_ARCH=-m32

appmetro: libappmetro.a item.o
	$(CC) $(TARGET_ARCH) $(CPPFLAGS) -o appmetro \
	-L../code \
	-lappmetro
	
clean:
	rm appmetro

VPATH=../code
