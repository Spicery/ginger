File2GNX
========

It should utilise a configurable file mapping. So where should that live? See..

http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html

Until version 0.8.1, we merely want to be compatible with that spec. This
implies that we should reference::

	/etc/xdg/ginger/parser-mapping.mnx

(N.B. This means adding another location to the install/uninstall functions.)

The contents should be a lookup table with the format.

.. code-block: XML

	<parser.mapping>
		<parser ext="cmn" exe="/usr/local/libexec/ginger/common2gnx"/>
		<parser ext="cst" exe="/usr/local/libexec/ginger/cstyle2gnx"/>
		<parser ext="lsp" exe="/usr/local/libexec/ginger/lisp2gnx"/>
	</parser.mapping>
