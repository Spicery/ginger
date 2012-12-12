#!/bin/bash

#
# Change the following line if you want Ginger installed somewhere else.
#
INSTALL_DIR=/usr/local

#
#	Install the packed files archive into /usr/local. We assume that the
#	packed files archive is in the current directory. 
#	N.B. You will probably need to run this script with root privileges.
#
PACKED_FILES=ginger-files.tgz
gzip -d ${PACKED_FILES} | ( cd ${INSTALL_DIR}; tar zxf -)

#
# Now create the uninstaller (uninstall.sh) in the share directory.
# Arguably it could go in the libexec.
# 
UNINSTALL_FILE=${INSTALL_DIR}/share/ginger/uninstall.sh
touch ${UNINSTALL_FILE}
echo '#!/bin/sh' > ${UNINSTALL_FILE}
echo "/bin/rm $(INSTALL_DIR}/bin/ginger" >> ${UNINSTALL_FILE}
echo "/bin/rm $(INSTALL_DIR}/bin/ginger-*" >> ${UNINSTALL_FILE}
echo "/bin/rm -rf $(INSTALL_DIR}/libexec/ginger" >> ${UNINSTALL_FILE}
echo "/bin/rm -rf $(INSTALL_DIR}/share/ginger" >> ${UNINSTALL_FILE}
chmod a+x ${UNINSTALL_FILE}

INSTALL_INFO_FILE=${INSTALL_DIR}/share/ginger/INSTALL_INFO.txt
touch ${INSTALL_INFO_FILE}
echo "Ginger Installation from Tarball Information" >> ${INSTALL_INFO_FILE}
echo "--------------------------------------------" >> ${INSTALL_INFO_FILE}
echo "date         : " `date` >> ${INSTALL_INFO_FILE}
echo "uname -mn    : " `uname -mn` >> ${INSTALL_INFO_FILE}
echo "uname -v     : " `uname -v` >> ${INSTALL_INFO_FILE}
chmod a+r ${INSTALL_INFO_FILE}
