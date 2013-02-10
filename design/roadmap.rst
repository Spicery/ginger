Ginger Release Roadmap
======================

.. include:: version_0.8.rst

.. include:: version_0.8.1.rst

Version 0.9
-----------

Basic Language Features
~~~~~~~~~~~~~~~~~~~~~~~

* Binding to multiple variables.
* Assignment to multiple variables.
* Anonymous variables.
* Final classes (without inheritance).
  - Slots
  - Constructors
  - Recognisers

Novel Language Features
~~~~~~~~~~~~~~~~~~~~~~~

* Partial application with ? syntax.

Common Syntax
~~~~~~~~~~~~~

* Curried definitions and lambdas.
* --pythonic: supports the python-style indentation that Graham and I discussed.


Basic Library
~~~~~~~~~~~~~

* Equality
* Simple file based I/O
* valueClass works for all basic values
* print and show works for all baic values

Ginger
~~~~~~

* Traps keyboard-interrupts
* Uses a user/team/department/.../world preferences hierarchy 
* Provides the context of the current project and package

  * Autolocates the current project 
      - current directory
      - --project option
      - ~/.config/ginger preferences

  * Auto-imports the packages of the current project into the interactive
    package.

  * Special command ‘macros’
      - !<unix command> - run a unix command in a subshell
      - help - integrated help - launches a web-browser? - using javascript for dynamic search?
      - imports - edits interactive imports
      - inspect - inspects variables (or the results of the last computation)



Ginger #! Script Tool
~~~~~~~~~~~~~~~~~~~~~

* Additionally supports being passed multiple scripts in Common syntax.
    % ginger-script script1.cmn script2.cmn script3.cmn
* Additionally supports executing from the standard input in any 
  supported syntax.

    % echo 'println( "hello world" );' | ginger-script -i -g common



Version 0.10: Delay-Force Sprint
--------------------------------

This version is dedicated to the introduction of explicit delay + implicit 
forcing. After some thought I realised this will be a significant challenge 
and hence it deserves its own version.

Novel Language Features
~~~~~~~~~~~~~~~~~~~~~~~

* Explicit delay, implicit forcing.

Version 0.11
------------

* Adding support for lisp-like front-end syntax (lisp2gnx)
* Adding idiomatic names for the different language front-ends.
* Adding support for indentation-controlled syntax (common2gnx)

Version 0.12: Unicode Sprint
----------------------------

* A possible pre-1.0 version that will support Unicode BMLP.

Release 1.0
-----------

Platforms
~~~~~~~~~

* Linux: Debian (Ubuntu), Redhat (Fedora), OpenSUSE.
* Mac OS X
* Free BSD

Basic Language Features
~~~~~~~~~~~~~~~~~~~~~~~

* Single inheritance classes.
* Single dispatch methods.
  
Novel Features
~~~~~~~~~~~~~~

* First class virtual machines.

Documentation 
~~~~~~~~~~~~~

  * a overview of the Ginger project, the manifesto, the motivation, the roadmap. [todo]
  * all the tools in the toolchain [todo]
  * all the source formats (Common syntax, C-style syntax, GingerXML, MinimalMnx) 
    used in the toolchain. [todo]
  * the programs (ginger, ginger-cgi, ginger-script) [todo]
  * the Ginger language features [todo]
  * the Ginger classes and standard library [todo]
  * Integration with the Sublime 2 text editor [todo]
  

Ginger CGI
~~~~~~~~~~
 
* Additionally supports scripts invoked through Apache Action/AddHandler.
* Additionally supports apache CGIMapExtensions.
* Supports file upload.

Release 2.0
-----------

Basic Language Features
~~~~~~~~~~~~~~~~~~~~~~~

* Keyword arguments with default values (via the -with- keyword)
* Vectorclasses.
* Multiple inheritance.
* Essences.
* Multiple dispatch.

Novel Features
~~~~~~~~~~~~~~

* Alternative-returns (via the return keyword) escalating to panics.

Standard Library
~~~~~~~~~~~~~~~~

* File i/o.


Ginger
~~~~~~

* admin macro (or command?)
  - create new project folder (optionally make current)
  - unset/set/change current project (by folder)
  - remove current project
  - delete current project folder
  - query current project
  - create/edit/delete/query package in current project


Future Releases
---------------

Novel Features
~~~~~~~~~~~~~~

* Rollbacks.

Ginger
~~~~~~

  * Long term development would be to become a terminal-based Project IDE suitable 
    for use over SSH.
  * Emacs support.
