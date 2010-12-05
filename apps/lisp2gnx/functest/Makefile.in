
#	Do nothing
all:
	
#	This is the top-level make
check: lisp.pass

.tests.pass:
	java -cp ../../../classes com.steelypip.appginger.functest.FuncTestMain "guile -l ../lsp/lisp2gnx.lsp -e lisp2gnx" $< 
	
clean:
	rm -f *.pass
	
.SUFFIXES: .pass .tests
