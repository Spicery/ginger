Standard Exception Types
========================

Assertion Failure
-----------------
These are raised when an assertion made by the programmer that must evaluate to true, evaluates
to false.

  * AF1: Assertion failure - raised when an assert statement fails.


Bad Arguments
-------------

The basic situations are:

  * BA1: Function never handles argument of this type - an argument has been supplied that
    does not meet the up-front (static) type requirements of the parameters. e.g. '+' works
    on numbers and nothing else.

  * BA2: Function never handles argument with this value - an argument has been supplied that
    does not meet the up-front (static) value requirements of the parameters. e.g. Finding
    the median value of an empty list. Division by zero would be another case if it wasn't
    for transreal values.

  * BA3: Function can't handle argument with this value at this time - an argument has been supplied 
  	that does not meet the dynamic requirements on the parameters. e.g. Searching for an item
  	in a list that does not contain the item.

Resource Exhaustion
-------------------

  * RE1: Satisfying the request would exceed a limit that can be reconfigured in Ginger, typically
    at startup. A hint as to how to reconfigure it will be supplied. e.g. exceeding recursion 
    limit.

  * RE2: Satisfying the request would exceed a limit outside of Ginger. A hint of how to improve
    the situation may be supplied.
    e.g. memory exhaustion. e.g. running out of file descriptors.


Plumbing Problems (aka System Error)
------------------------------------

  * PP1: The Ginger-Runtime has dynamically discovered an issue that means the program does not
    meet the runtime rules. Although we try to eliminate all such situations, these may exist.
    e.g. discovering that a plug-in has not correctly loaded.

  * PP2: The Ginger-Runtime has dynamically discovered an issue with the program that means that
    part of the program is invalid but this evaded the compiler.

Mapping Python exceptions onto this structure
=============================================

As a sanity check, here's a mapping from the different types of exceptions in Python3 (which is 
a practical programming language with a lot of shared goals) into Ginger's classification.

AssertError - AF1
AttributeError - BA1
EOFError - an escape
GeneratorExit - an escape
ImportError/ModuleNotFoundError - a compiler error or PP1
IndexError - BA3 (likely to be an escape)
KeyError - BA3 (likely to be an escape)
KeyboardInterrupt - not an exception but an 'engine' switch
MemoryError - RE1 or RE2 depending
NameError - compiler error
NotImplementedError - PP1
OSError - unclassifiable
OverflowError - BA2
RecursionError - RE1
ReferenceError - an escape, not an exception
RuntimeError - an abstract exception class
StopIteration - not an exception but an escape
StopAsyncIteration - not an exception but an escape
SyntaxError - a compiler error
IndentationError - a compiler error
TabError - a compiler error
SystemError - PP1
SystemExit - not an exception but a engine switch
TypeError - BA1
UnboundLocalError - PP2
UnicodeError - BA2 - also UnicodeEncodeError, UnicodeDecodeError
ValueError - BA2
ZeroDivisionError - does not arise as we use transreals




