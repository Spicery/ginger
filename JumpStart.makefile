.PHONY: all
all:
	# Targets
	#	ubuntu - installs prerequisites for ubuntu
	#	configure - builds all makefiles via autoconf
	#	clean - cleans down completely
	# 	build - clean build
	#	install - clean install

.PHONY: ubuntu
ubuntu:
	bash devtools/docker-scripts/install-prerequisites.bsh

define runconfigure
	autoconf
	./configure
endef

.PHONY: configure
configure:
	$(call runconfigure)

.PHONY: build
build: clean
	$(call runconfigure)
	make

.PHONY: install
install: build
	sudo make install-as-is

.PHONY: clean
clean:
	if [ -e Makefile ]; then make clean; make distclean; fi
