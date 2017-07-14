.PHONEY: all
all:
	# Targets
	#	ubuntu - installs prerequisites for ubuntu

.PHONEY: ubuntu
ubuntu:
	bash devtools/docker-scripts/install-prerequisites.bsh

.PHONEY: configure
configure:
	autoconf
	./configure

.PHONEY: clean
clean:
	make clean
	make distclean
