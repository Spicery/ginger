Welcome to Ginger
=================

Ginger is our attempt to build an elegant but friendly and practical programming system. It includes a programming language, virtual machine and supporting tools.

The programming language itself is syntax-neutral. By this we mean that you can choose any of several 'pluggable' front-end syntaxes - there's one rather like Algol/Pascal, another like Javascript, another like Lisp. They all get turned into an elegant back-end XML syntax.

Ginger is a modern programming language supporting automatic memory management, object oriented programming, pervasive multiple values and a simple but powerful data loading mechanism that includes data-format conversion.

Ginger is engineered to run on UNIX systems and is developed on OS X and Linux. Adapting it for other UNIXs is straightforward though.

How to Install
--------------

* Follow the instructions in `INSTALL`__

.. __: https://raw.github.com/Spicery/ginger/master/INSTALL

How to Get Started
------------------

Execute the ginger command in a shell::

    % ginger


Contents
--------

INSTALL.txt
    How to make and install AppGinger.

README.rst
    This file.

apps/automatic
    Automatically generated C++ files.

apps/appginger
    Source for Ginger Virtual Machine

apps/lisp2gnx
    Source for Lisp front end. (Work in progress.)

apps/common2gnx
    Source for Common Syntax front end.

design
    Developer's design notes. Not intended for programmers but contributors
    to the Ginger implementation.

examples
    Examples of Common syntax (out of date).

instruction_set
    Source code for instructions, C++ 

