HTML_FILES=welcome.html \
	appginger.html \
	common_syntax.html \
	overview.html \
	get_started.html \
	gnx_syntax.html \
	gson_background.html \
	gson2gnx.html \
	gson.html \
	lnx.html \
	lnx2mnx.html \
	mnx_syntax.html \
	patterns_and_queries.html \
	toolchain.html

RAW_FILES=\
	ginger_loops.odt \
	ginger_xml_syntax.odt \
	packages.odt \
	ginger_overview.odt \
	lisp_syntax.odt

all: $(HTML_FILES)

clean:
	/bin/rm -f $(HTML_FILES)
	rm -rf build/documentation
	rm -f build/documentation.tgz

tarball: all
	mkdir -p build
	rm -rf build/documentation
	mkdir -p build/documentation
	cp $(HTML_FILES) build/documentation
	cp $(RAW_FILES) build/documentation
	cp -r images build/documentation/
	find build -name '.DS_STORE' -print0 | xargs -0 rm -f
	find build -type d -name '.svn' -print0 | xargs -0 rm -f
	( cd build; tar cf - documentation | gzip > documentation.tgz )
	
%.html: %.txt
	asciidoc -a icons --attribute=tabsize=4 -n $<
