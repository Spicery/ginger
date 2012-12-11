#!/bin/bash
#
#	Install the packed files archive into /usr/local. We assume that the
#	packed files archive is in the current directory. 
#	N.B. You will probably need to run this script with root privileges.
#
PACKED_FILES=ginger-files.tgz
INSTALL_DIR=/usr/local
gzip -d $PACKED_FILES | ( cd $INSTALL_DIR; tar zxf -)
