all:
	$(MAKE) -C cpp $@
	
dox:
	doxygen Doxyfile

clean:
	$(MAKE) -C cpp $@
	$(MAKE) -C unittests $@

check:
	$(MAKE) -C unittests $@
