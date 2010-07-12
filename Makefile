all:
	$(MAKE) -C c $@ 
	if [ ! -d bin ]; then mkdir -p bin; fi
	cp c/appginger/cpp/appginger bin/
	cp c/common2gnx/cpp/common2gnx bin

clean:
	rm -rf build/appginger
	rm -f bin/common2gnx
	rm -f bin/appginger
	$(MAKE) -C c $@
	
check: all
	$(MAKE) -C c $@


# This will become dist & distdir??
snapshot:
	rm -rf build/appginger
	mkdir build/appginger
	cp Makefile build/appginger/
	cp *.txt build/appginger/
	mkdir build/appginger/bin
	mkdir build/appginger/c
	cp -r c/Makefile build/appginger/c/
	cp -r c/appginger build/appginger/c/
	cp -r c/headers build/appginger/c/
	cp -r c/automatic build/appginger/c/
	cp -r c/common2gnx build/appginger/c/
	mkdir build/appginger/classes
	mkdir build/appginger/docs
	cp -r docs/syntax/*.txt build/appginger/docs/
	cp -r examples build/appginger/
	cp -r instructions build/appginger/
	cp -r src build/appginger/
	( cd ./build ; tar cf - appginger | gzip > appginger.tgz )
