all:
	$(MAKE) -C cpp $@
	
dox:
	doxygen Doxyfile

clean:
	$(MAKE) -C cpp $@
	$(MAKE) -C functests $@

check:
	$(MAKE) -C functests $@
