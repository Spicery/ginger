%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
fn2code: Compiling fn elements to code elements
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

================================================================================
The Vision
================================================================================

The aim is to have a new tool, a helper application ``fn2code``, that
is an integral part of the Ginger toolchain. This new tool will process 
GingerXML by translating the tree-like contents of ``fn`` GingerXML into 
a ``fn.code`` element whose children are an instruction list.

For example::

    define f( x, y ) =>> x + y enddefine

Generates::

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="ginger.interactive" name="f" protected="true" scope="global" uid="0"/>
        <fn arity.eval="1" name="f">
            <seq arity.pattern="2">
                <var arity.pattern="1" name="x" protected="true" scope="local" span="1" uid="1"/>
                <var arity.pattern="1" name="y" protected="true" scope="local" span="1" uid="2"/>
            </seq>
            <sysapp args.arity="2" arity.eval="1" name="+">
                <id arity.eval="1" name="x" protected="true" scope="local" span="1" uid="1"/>
                <id arity.eval="1" name="y" protected="true" scope="local" span="1" uid="2"/>
            </sysapp>
        </fn>
    </bind>

The "fn" element should be turned into a "fn.code" element that contains an instruction list::

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="ginger.interactive" name="f" protected="true" scope="global" uid="0"/>
        <fn.code args.count="2" arity.eval="1" locals.count="2" arg.count="2" name="f">
            <enter/>
            <push.local local="0"/>
            <push.local local="1"/>
            <add/>
            <return/>
        </code>
    </bind>

Or, with suitable optimisation, into::

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="ginger.interactive" name="f" protected="true" scope="global" uid="0"/>
        <fn.code args.count="2" arity.eval="1" locals.count="2" arg.count="2" name="f">
            <check count="2"/>
            <add/>
            <sysreturn/> 
        </code>
    </bind>


================================================================================
Acceptance Criteria
================================================================================

Integration with Codebase
-------------------------
  * A new tool whose C++ source code resides in ``${GINGER_DEV_HOME}/apps/fn2code``.
  * When compiled, the tool creates a native executable ``${GINGER_DEV_HOME}/apps/fn2code/fn2code``.
  * When installed, the tool's executable is placed in ``${GINGER_HOME}/libexec/ginger/fn2code``.

Features
--------
  * The ``fn2code`` will take output from simplifygnx that and replace
    the ``fn`` elements with ``fn.code`` elements. 
  * It will not generate jump-to-jumps.
  * It will support tail-call optimisation (TCO) by generating chain 
    calls rather than direct calls. This may require additional VM instructions.
  * The instructions to be used by ``fn.code`` will be documented as part of 
    the instruction set files. [This has already been done 
    e.g. see ``${GINGER_DEV_HOME}/instruction_set/and.i``]
  * The input and output will be in the custom format `Minimal XML`_. 
  * The tool will be a command-line tool and process its command-options
    consistently with the other tools (i.e. in GNU long options style.)
  * The Ginger Runtime (``libappginger.a``) will behave exactly the same on the
    replaced ``fn.code`` elements as it would have on the ``fn`` elements. 
  * A new option, --fn2code available on ginger-cli, ginger-script and 
    ginger-cgi, that enables/disables the new tool in the toolchain. 
    Enabling this option should have no functional effect - although 
    performance of compilation and run-time may vary a little.
  * A new compilation option for libappginger.a that disables the 
    recognition of ``fn`` elements and all the supporting code.
    If this option is enabled the --fn2code option is ignored.

Documentation
-------------
  * The Ginger Docs will be updated with a manual page describing the
    usage of the new tool.
  * A technical note will be included in the ``${GINGER_DEV_HOME}/design``.
  * Both will be authored in ReStructured text (``\*.rst``).

Standards (Definition of Done)
------------------------------
  * Indentation of C++ source code will be consistent with the other
    tools and be based on 1TB style. The house style is called
    balanced indentation and is described here 
    http://steelypip.wikidot.com/balanced-indentation
  * Documentation is written in `ReStructured Text`_.


Testing
-------
  * A new test folder, ``${GINGER_DEV_HOME}/functests/fn2code`` will contain component
    integration tests for the new ``fn2code`` tool. It may assume that there is 
    an up-to-date Ginger installation and use any of the other installed tools,
    except itself.
  * The tests will typically take source code in GNX or Common syntax,
    parse, resolve, simplify and then run through fn2code and verify the
    output.
  * All the system tests will run with both --fn2code enabled and disabled.


================================================================================
Process
================================================================================

Supporting Documents
--------------------

  * This work is part of `Sprint 0.9.3`_ - although the word Sprint is 
    used a little loosely. (Perhaps 'Advance' would be better than 'Sprint'?)

  * The GingerXML_ (or GNX) syntax is described in the Ginger Docs repository
    at ``${GINGER_DEV_DOCS}/format/gnx_syntax.rst``. This is a key document.
    To generate GingerXML it is handy to use one of the front-end syntaxes
    such as Common_ - although the documentation on the latter could do
    with a lot of improvement (and could reasonably be a story we take on
    in this cycle.)

  * The instruction set is not directly documented. Instead each instruction
    is represented by a file in the folder ``${GINGER_DEV_HOME}/instruction_set``.
    The header of each file is (at a stretch) self-documenting. 

    I have made a somewhat clumsy attempt to generate documentation but the 
    integration with the rest of the system is very unsatisfactory. In lieu
    of getting that right, I have attached a PDF of the instruction set docs 
    to the Trello epic.

  * The Ginger Runtime (aka appginger) currently implements a compiler for GNX
    into the instruction set - so that's a fairly useful resource, in so
    far that it provides a template for how this can be done. The goal of this
    epic is to rip out that code from the Ginger Runtime, of course.

  * A description of the `MinXML`_ syntax is provided on the Steelypip wiki.

  * A handy list of well-known folders is provided in 
    ``${GINGER_DEV_HOME}/design/envvars.rst`` (or `online`_)

  * The `Balanced Indentation`_ style is described on the Steelypip wiki.

  * Documentation in written in the lightweight markup of `ReStructured Text`_.

  * Development practices are described in on the `Ginger Dev Practices`_
    Trello board.

Supporting Tools
----------------
I envisage the main tools that will be useful, at least at first, will
be the parser (for Common_), the simplifier and the pretty printer. You
use these in a pipeline like this - where /usr/local is shorthand for
${GINGER_HOME}.

.. code-block:: bash

    cat FILE | \
    /usr/local/libexec/ginger/common2gnx | \
    /usr/local/libexec/ginger/simplifygnx -suA | \
    /usr/local/libexec/ginger/tidymnx

For example, we can try compiling a simple function that doubles a number
then adds one. For example, the following command 

.. code-block:: bash

    echo 'define f( x ) =>> 2 * x + 1 enddefine;' | \
    /usr/local/libexec/ginger/common2gnx | \
    /usr/local/libexec/ginger/simplifygnx -suA | \
    /usr/local/libexec/ginger/tidymnx

will generate the slightly intimidating output ...

.. code-block:: xml

    <bind arity.eval="0" span="1">
        <var arity.pattern="1" def.pkg="" name="f" protected="true" scope="global" uid="0"/>
        <fn args.count="1" arity.eval="1" locals.count="1" name="f">
            <seq arity.pattern="1">
                <var arity.pattern="1" name="x" protected="true" scope="local" slot="0" span="1" uid="1"/>
            </seq>
            <sysapp args.arity="2" arity.eval="1" name="+">
                <sysapp args.arity="2" arity.eval="1" name="*">
                    <constant arity.eval="1" span="1" type="int" value="2"/>
                    <id arity.eval="1" name="x" protected="true" scope="local" slot="0" span="1" uid="1"/>
                </sysapp>
                <constant arity.eval="1" span="1" type="int" value="1"/>
            </sysapp>
        </fn>
    </bind>

Branch Discipline
-----------------
There are two persistent branches in the Ginger project, namely master and 
development. Master is our release branch and each push to master must be
version tagged (using `semantic versioning`_). In addition the version number
of Ginger (defined in ${GINGER_DEV_HOME}/configure.ac) must be consistent
with the tag.

Development is the branch representing the current sprint's work and the
version number should have the suffix "-dev". At the time of writing it
is 0.9.3-dev, for instance. 

We will do this work in a feature branch ``fn2code`` which I have already
pushed to GitHub.

Two types of pushes are permitted to ``development`` and feature branches. Normal
pushes are done when the code compiles and is locally tested using make check. 
But because this is a hobby activity squeezed into odd moments, sometimes it
is necessary to make an untested work-in-progress (WIP) commit. The commit 
comment for such a push *must* be marked with the code "WIP" at the start of the 
message. Furthermore, if there are any code changes it must be marked so as
not to trigger Travis, the continuous integration system by adding ``[ci skip]``
into the commit comment.

On a release to ``master`` the code is always fully tested locally in the
development branch plus verifying the Travis build before merging into the 
master branch. Then the master branch is locally and fully retested in a 
clean environment. 

Each epic or user story should be worked on in a separate feature branch. 
This work package (epic) will be done on the fn2code branch. Changes that
need to be shared across feature branches are made on the development 
branch and merged into all the current feature branches.


First, Prototype in Python
--------------------------
This is just a suggestion - but I think commonsense. It's not easy to solve 
an unfamiliar problem while programming in an unfamiliar language. So I 
recommend doing a prototype in Python first.

The only real stumbling block will be parsing MinXML_, so I have written
a Python3 `module minxml`_. That library is technically still in development
but it is nearly at the end of the development cycle. The main shortcoming
is that I haven't implemented the doc-comments. However the unit tests give
plenty of examples how to use it.

This work will be done inside a new folder in the ``apps`` directory,
${GINGER_DEV_HOME}/apps/fn2code. I suggest this prototype should be implemented 
in a subfolder called (say) ``prototype``. For the prototype we will assume that 
python3 is installed and set about installing the python scripts in 
${GINGER_SHARE}.

For this work you want to read a series of GingerXML_ expressions on the 
input, transform them, and emit them as GingerXML_ on the output. So the
key file fn2code.py will look something like:

.. code-block:: Python3

    #!/usr/bin/env python3

    import minxml
    import sys

    def doSomeTransformation( gnx ):
        # Processing goes here.
        return gnx

    def main():
        while True:
            gnx = minxml.readMinXML( sys.stdin )
            if gnx == None:
                break
            gnx = doSomeTransformation( gnx )
            print( gnx )

    if __name__ == "__main__":
        main()


In parallel I will start implementing the new flags and compiler on the 
Ginger Runtime so that we can system-test the prototype. And I have coded up
the install script for the prototype too. By default the prototype gets
put in ``${GINGER_SHARE}/fn2code/*.py``.


Then, Implement in C++
----------------------
Once we have got the prototype working nicely, it can be turned into the 
core of a unit test for the C++ code. 

The C++ applications have got quite a set style to the way they are written.
I haved created the skeleton for the C++ version, including editing the 
make scripts, which isn't intuitive. This skeleton currently just invokes the 
installed prototype.


Dividing Up Tasks
-----------------
- LH: Python prototype and unit tests
- LH: C++ version
- LH: Tech note on implementation (in the design folder)
- LH: Programmer guide (on Gingerdocs)
- SFKL: minxml.py [done]
- SFKL: C++ skeleton [done]
- SFKL: Install scripts [done]
- SFKL: Modify Ginger Runtime to implement new flags
- SFKL: Add any new chain instructions.

  
.. _Sprint 0.9.3: https://trello.com/b/a60qNt0K/ginger-sprint-093
.. _GingerXML: http://ginger.readthedocs.io/en/latest/formats/gnx_syntax.html
.. _Common: http://ginger.readthedocs.io/en/latest/syntax/common_syntax.html
.. _MinXML: http://steelypip.wikidot.com/minimal-xml
.. _Minimal XML: MinXML_
.. _online: https://github.com/Spicery/ginger/blob/development/design/envvars.rst
.. _Balanced Indentation: http://steelypip.wikidot.com/balanced-indentation
.. _ReStructured Text: http://docutils.sourceforge.net/rst.html
.. _semantic versioning: http://semver.org/
.. _Ginger Dev Practices: https://trello.com/b/qk0KWBd7/ginger-dev-practices
.. _module minxml: https://github.com/sfkleach/MinXML/blob/dev/python3/minxml.py





