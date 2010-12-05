# --- Paths ---
# EPREFIX should be bound to PREFIX by default. This appears to be a lie.
# As a consequence we override EPREFIX entirely. Probably incorrect but not
# that interesting.
prefix=@prefix@
exec_prefix=@prefix@
datarootdir=@datarootdir@
PACKAGE_TARNAME=@PACKAGE_TARNAME@
INSTALL_BIN=@bindir@
INSTALL_LIB=@datarootdir@/@PACKAGE_TARNAME@
INSTALL_DOC=@docdir@
