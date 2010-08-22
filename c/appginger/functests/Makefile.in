
#	Do nothing
all:
	
#	This is the top-level make
check: smoke.pass literals.pass

.tests.pass:
	java -cp ../../../classes com.steelypip.appginger.functest.FuncTestMain $< 
	
clean:
	rm -f *.pass
	
.SUFFIXES: .pass .tests
