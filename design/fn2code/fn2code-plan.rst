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
  * A new tool whose C++ source code resides in ${GINGER_DEV_HOME}/apps/fn2code.
  * When compiled, the tool creates a native executable ${GINGER_DEV_HOME}/apps/fn2code/fn2code.
  * When installed, the tool's executable is placed in ${GINGER_HOME}/libexec/ginger/fn2code.

Features
--------
  * The ``fn2code`` will take output from simplifygnx that and replace
    the ``fn`` elements with ``fn.code`` elements. 
  * The instructions to be used by ``fn.code`` will be documented as part of 
    the instruction set files. [This has already been done 
    e.g. see ${GINGER_DEV_HOME}/instruction_set/and.i]
  * The input and output will be in the custom format Minimal XML. This
    is described on http://steelypip.wikidot.com/minimal-xml. 
  * The tool will be a command-line tool and process its command-options
    consistently with the other tools (i.e. in GNU long options style.)
  * The GingerVM (libappginger.a) will behave exactly the same on the
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
  * A technical note will be included in the ${GINGER_DEV_HOME}/design.
  * Both will be authored in ReStructured text (\*.rst).

Standards (Definition of Done)
------------------------------
  * Indentation of C++ source code will be consistent with the other
    tools and be based on 1TB style. The house style is called
    balanced indentation and is described here 
    http://steelypip.wikidot.com/balanced-indentation

Testing
-------
  * A new test folder, ${GINGER_DEV_HOME}/functests/fn2code will contain component
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

Supporting Documents and Tools
------------------------------


Branch Discipline
-----------------

Prototype in Python
-------------------


Implementation in C++
---------------------
  





