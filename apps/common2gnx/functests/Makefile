#	No need for a Makefile.in when it is this simple.

#	Do nothing
all:

#	This is the top-level make
check: example.pass envvar.pass element.pass

.tests.pass:
	java -cp ../../../classes com.steelypip.appginger.functest.FuncTestMain "../cpp/common2gnx" $< 
	
clean:
	rm -f *.pass
	rm -f *.fail
	
.SUFFIXES: .pass .tests
