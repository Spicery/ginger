Ginger Version 0.8
------------------

Platforms
~~~~~~~~~

Version 0.8 will build, install and run tests on the following:

  * Linux (Ubuntu)
  * Mac OS X

Installation will be from

  * Source - via git clone, build and install
  * Tarball

Syntaxes Supported
~~~~~~~~~~~~~~~~~~

There will be two front-end syntaxes supported.

  * Common - our custom syntax designed to meet the goals of the manifesto.
  * C-Style - a C/Java style syntax that Javascript programmers may find more
    approachable.

Basic Language Features
~~~~~~~~~~~~~~~~~~~~~~~

*  binding and assignment to single variables
*  if/unless N-way conditionals and switch expressions
*  short circuit conditionals (boolean & absent-oriented)
*  function applications and partial applications
*  higher-order functions with full lexical scoping
*  undecorated throws which are not catchable (escalated to failover)
*  garbage collection
*  autoloading and autoconversion
*  Environment variables via ${NAME} syntax.

Basic Datatypes
~~~~~~~~~~~~~~~

This release of Ginger comes with the following datatypes built-in.

*  simple primitives: absent, booleans, undefined, indeterminate.
*  small integers and small integer arithmetic, overflowing to floating point.
*  floating point arithmatic.
*  ASCII strings, symbols and characters
*  singly linked lists (Lisp-style)
*  1D arrays (vectors)
*  maps and maplets, including weak hash tables
*  class objects
*  user-defined recordclasses

Ginger Executable
~~~~~~~~~~~~~~~~~

A command-line interpreter, ginger, that coordinates the toolchain components to
provide an interactive shell for exploratory programming in a terminal. [design+docs+implement]

* Simple, single-line, terminal based interpreter.
  - Uses readline by default to provide command line editing.
  - Consumes stdin.
  - Interactive.
* Exceptions always escalate to failover.
* Traps failovers, reports them, resets, continues.
* Panics abort the system (not sure this is correct).
* Prints welcome banner.
* Selectable syntax via -~grammar option.
* Starts up in ginger.interactive package.


Ginger CGI Executable
~~~~~~~~~~~~~~~~~~~~~

A CGI script runner, ginger-cgi (o.n.o.), that can be used to write CGI scripts.
This is very much just a placeholder implementation. Future versions will
extend this capability very significantly.

* supports #!/usr/local/bin/ginger-cgi (hash-bang) scripts.
* can be used interactively for testing.


Ginger #! Script Executable
~~~~~~~~~~~~~~~~~~~~~~~~~~~

A scripting tool, ginger-script, that can be used to write UNIX scripts. 

* Immediately exits on error.
* Supports hash-bang scripting #!/usr/local/bin/ginger-script.

Toolchain
~~~~~~~~~

Additional tools that provide modular functionality. Over time the user
will be able to add replacement or extensions to these. They are not 
supposed to be on the $PATH of a typical user. By default they are installed 
in /usr/local/libexec/ginger

* common2gnx:     a front-end for Common, converts to GingerXML.
* cstyle2gnx:     a front-end for a C-family syntax, converts to GingerXML.
* file2gnx:       a tool for parsing a file based on the file extension.
* simplifygnx:    a GingerXML simplifier and annotation tool. 
* tidymnx:        a tool for pretty-printing MinXML (superset of GingerXML).
* fetchgnx:       a tool for accessing Project contents.
* ginger-cli:     a basic Ginger command-line interpreter.

