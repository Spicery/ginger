.PHONEY: all
all:
	# Targets
	#	ubuntu - installs prerequisites for ubuntu
	#	configure - builds all makefiles via autoconf
	#	clean - cleans down completely
	# 	build - clean build
	#	install - clean install

.PHONEY: ubuntu
ubuntu:
	bash devtools/docker-scripts/install-prerequisites.bsh

define runconfigure
	autoconf
	./configure
endef

.PHONEY: configure
configure:
	$(call runconfigure)

.PHONEY: build
build: clean
	$(call runconfigure)
	make

.PHONEY: install
install: build
	sudo make install-as-is

.PHONEY: clean
clean:
	if [ -e Makefile ]; then make clean; make distclean; fi
