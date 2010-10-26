Welcome to AppGinger
====================

How to Install
--------------

  * Follow the instructions in INSTALL.txt


How to Get Started
------------------

  * cd $INSTALL
  * ./bin/common2gnx | ./bin/appginger


Contents
--------

    INSTALL.txt     How to make and install AppGinger.
    README.txt      This file.
    TO_DO.txt       To do list.
    bin             Where the appginger & common2gnx binaries will be created.
    c/automatic     Automatically generated C++ files.
    c/appginger     Source for appginger interpreter appginger.
    c/lisp2gnx      Source for Lisp front end.
    c/common2gnx	Source for Common Syntax front end.
    classes         Java class files used to generate C++ code.
    docs            Documents
    examples        Examples of Common syntax (out of date).
    instructions    Source code for instructions, C++ 
    src             Java source files for Code Generator
    				and the functional tests.

Notes for Developers
--------------------

I thought it was about time I jotted down how the AppGinger folder
hierarchy is organised and the non-standard build process. The
prerequisites are UNIX (or Cygwin at a pinch), an up-to-date JDK, GNU
C++ and a Subversion client. I do everything on the command-line, so
nothing fancy is needed.

When you download from
http://svn6.assembla.com/svn/ginger/appginger/trunk/ you'll end up with
a rather intimidating mess. Let's start by downloading into a fresh
AppGinger folder and running configure.

	% svn checkout http://svn6.assembla.com/svn/ginger/appginger/trunk/ AppGinger
	% cd AppGinger
	% export APPGINGER=`pwd`
	% ./configure

Looking inside this folder you'll see a lot of files. The three key
folders here are:

	instructions
		The files in this folder are C++ fragments. These instructions are spliced
		together in 3 different ways to form the 3 different Virtual Machine C++ classes.

	src
		The Java code resides here. There are two programs. One does the code-generation
		and lives in src/com/steelypip/appginger/codegen. The other does the functional
		tests and lives in src/com/steelypip/appginger/functest.

	c
		The application sources for appginger, common2gnx and lisp2gnx reside here
		in c/appginger, c/common2gnx and c/lisp2gnx respectively! The vast majority
		of the work is done in c/appginger.

		It's a rather unfortunately named folder (it used to only contain C code).
		At some point it should be renamed "apps".

The first step in the build process is the horrible one. You have to run
the Java-based code generator. At the moment I simply start up Eclipse,
my current Java IDE, and run the console application
com.steelypip.appginger.codegen.Main.main with no arguments. 

A minimalistic way to do this is to create a folder 'classes' for the
Java class files and then use javac and java.

	% mkdir classes 
	% ( cd src; javac -d ../classes com/steelypip/appginger/codegen/Main.java)
	% java -cp classes com.steelypip.appginger.codegen.Main

This overwrites (or creates) the C++ include files in the c/automatic
folder hierarchy. In actual fact I have added the generated include
files to the repository so that an ordinary "make" works. However if you
modify any instructions or do any low level work you'll need to re-run
this process explicitly. This is obviously a bit naff but changing the
instruction set is a relatively unusual event.

Having generated the "automatic" include files, the next step is to
build appginger and its testrunner. The appginger executable is, of
course, the central part of the system. It has the following layout:

	c/appginger/hpp
		C++ header files for the appginger executables. 

	c/appginger/cpp
		C++ module files. These rely on the header files. The make file in this
		folder builds both the libginger.a library AND the appginger executable.
		'make' takes about a minute to run.

		% cd $APPGINGER/c/appginger/cpp
		% make 
		% ./appginger # try out the executable

	c/appginger/unittest
		CppUnit unit test framework. It relies on the libginger.a library and the
		header files in c/appginger/hpp. The make file here builds the testrunner
		executable.

		% cd $APPGINGER/c/appginger/cpp
		% make
		% ./testrunner	# run the unit tests

	c/appginger/functests
		This is the functional test area. The functional tests rely on a Java
		program that we have to compile separately. Again, this is really quite naff
		but tidying up isn't high up my to-do list yet. The make script relies on
		the class files living in ../../../classes, which is why we built the
		class files there.

		% # Make the functional test Java program.
		% cd $APPGINGER
		% ( cd src; javac -d ../classes com/steelypip/appginger/functest/FuncTestMain.java)
		%
		% # Run the functional tests.
		% cd c/appginger/functests	
		% make clean	# run tests from scratch
		% make check
	
After this, we should build the common2gnx executable. This is a
relatively simple, if untidy, C++ program that I constructed by ripping
the parser out from an early version of the executable. It needs some
TLC. It has a less developed structure.

	c/common2gnx/hpp
		The C++ header files
	
	c/common2gnx/cpp
		The C++ modules files.

		% cd $APPGINGER
		% cd c/common2gnx/cpp
		% make

	c/common2gnx/functests
		Some functional tests.

		% cd $APPGINGER/c/common2gnx/functests
		% make clean
		% make check

And that is basically everything. You may wish to install these other
tools that I also have:

	* guile: to get lisp2gnx working
	* rlwrap (or similar): to provide GNU readline capability for appginger & common2gnx 
	* doxygen: to create javadoc-like API documentation (not worth bothering with yet tbh)
	* Eclipse (or similar e.g. IntelliJ): some kind of Java IDE.
	* A C++ aware text editor. I use BBEdit on Mac OS X and gedit on Ubuntu.
