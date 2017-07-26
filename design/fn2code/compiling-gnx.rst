%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Compiling GingerXML into GingerVM-Code
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Introduction
============
The Ginger virtual machine is word-coded and stack-based. 

Word-coded means that an instruction consists of an instruction word 
followed by one or more data words, where each word has the same width 
as a C++ (void \*). Although this isn't at all compact, it allows us
to experiment with radically different implementations of instruction-codes
ranging from simple integers, through pointers to native functions or 
objects to raw pointers that can be jumped to (TIL). It also allows us
to have a large, extensible instruction set with complicated instructions for
relatively specialised situations. 

This is a CISC approach rather than a RISC approach, of course. The thinking
behind this is to minimise the overhead of the interpreter loop by using
instructions that do a lot of (useful) work. The difficulty is that complex
instructions do not compose well and the result is a patchwork of general,
simple instructions that compose nicely and complicated, specialised 
instructions that are single use.

Being stack-based means that the arguments to functions are passed on the
stack. Ginger is unusual in having pervasive multiple values, so that it
is often very difficult for the compiler to statically determine the 
number of inputs and number of outputs. Being stack-based simplifies this
a great deal; the downside is that this is not especially efficient way
to pass arguments and requires dynamic checking of available space.

Example
=======
Before diving into details, here's a simple example of a function in
Common-syntax and the instructions it should compile into. The idea
is to provide an introduction to the general design of the instruction
set, before getting into the details of GingerXML. 

Our example function will test if a number is non-negative and its 
square is less than one.

.. code-block:: text

    define check( x ) =>>
        x >= 0 and x*x < 1
    enddefine;

The idea is that we will generate a ``<fn2code>`` elements, with attributes
``args.count`` and ``locals.count``, and whose children are a sequence of
instruction elements. The ``args.count`` attribute tells the runtime how
many arguments this function will pop from the stack on entry and the
``locals.count`` tells it how many local variables this function needs 
reserved in the call-stack frame.

Since this function only has one input ``x`` and everything else is done
using the stack, the number of args is 1 and so is the number of locals. Hence
we will generate:

.. code-block:: xml

    <fn.code args.count="1" locals.count="1">
        ... instructions ...
    </fn.code>

The first instruction we generate will be <enter1/>. This is a very specialised
instruction that builds a stack-frame for functions with one input and pops
the arguments off the stack into the frame. It must be 
the very first instruction as it uses offsets of the virtual-program-counter 
to find the locals-count. So our generated code will look like ...

.. code-block:: xml

    <fn.code args.count="1" locals.count="1">
        <enter1/>
        ... further instructions ...
    </fn.code>

The comparison x > 0 is tackled first. The simplest way to compile this is
with the stack-based code: push x, push 0, call greater than or equal to.

.. code-block:: xml

    <fn.code args.count="1" locals.count="1">
        <enter1/>
        <push.local0/>
        <push.constant type="int" value="0"/>
        <gte/>
        ... further instructions ...
    </fn.code>

Then we need to implement the short-circuit operator ``and``, which is
done via the ``and`` instruction. This instruction checks the top of the
stack and if it is false, short-circuits the rest of the computation by
jumping to the end. The only thing is we don't know how far to jump yet,
so we have to leave that blank.

.. code-block:: xml

    <fn.code args.count="1" locals.count="1">
        <enter1/>
        <push.local0/>
        <push.constant type="int" value="0"/>
        <gte/>
        <and to=END/>
        ... further instructions ...
    </fn.code>

Now we compute x*x > 0, although we will skip checking there's enough room
on the stack (that's an omission in the current design, based on the plan
to implement guard pages.) At this point the instructions should be self
explanatory - and must finish on a ``return`` instruction that tears down
the stack frame and returns to the caller.

.. code-block:: xml

    <fn.code args.count="1" locals.count="1">
        <enter1/>
        <push.local0/>
        <push.constant type="int" value="0"/>
        <gt/>
        <and to=END/>
        <push.local0/>
        <push.local0/>
        <mul/>
        <push.constant type="int" value="1"/>
        <lt/>
        <return/>
    </fn.code>

All that remains is to compute the distance that the ``and`` has to jump. The
jump has to skip two ``push.local0`` (2 x width 1), one ``mul`` (1 x width 1), a ``push.constant`` (1 x width 2), for a total of 6 words. There is also an
offset of 1 that has to be factored in (when the instruction is executed the
virtual-pc is pointing one past the start of the current instruction, which is
two words long.) So the value that has to be substituted is 6 + 1 = 7.

To assist with readability, some additional label attributes have been added to
create a complete solution.

.. code-block:: xml

    <fn.code args.count="1" locals.count="1">
        <enter1/>
        <push.local0/>
        <push.constant>
            <constant type="int" value="0"/>
        </push.constant>
        <gt/>
        <and to="7" to.label="L1"/>
        <push.local0/>
        <push.local0/>
        <mul/>
        <push.constant>
            <constant type="int" value="1"/>
        </push.constant>
        <lt/>
        <return label="L1"/>
    </fn.code>

Compiling GingerXML
===================

Here we breakdown how the current Ginger Runtime compiles the different
GingerXML elements - slightly adapted to be in the language of the new
InstructionXML.

constant
--------
Constants are simply pushed onto the stack, ignoring the necessity to check
for available room. The instruction that is set aside for this is the 
``pushq``. The name of this instruction isn't particularly logical - it
derives from a pattern established by the Poplog compiler - and it should
probably be renamed ``push.constant``. 

This instruction has a single child that is the constant
expression to be pushed e.g.

.. code-block:: xml

    <pushq> 
        <constant type="string" value="Hello, World!"/>
    </pushq>

N.B. InstructionXML is not particularly concise - and nor is GingerXML.
The aim of both formats is clarity and simplicity.

Variable References
-------------------
Variables are referenced (more colloquially 'used') using the ``id`` element.
By the time the compiler tool gets to see these elements they have been 
transformed into either *global* or *innermost local* variables. The category
that has been eliminated is local variable that are captured by inner 
lambda expressions; they are transformed into combinations of explicit 
capture and inner local variables. 

Global variables are marked as ``scope="global"`` and inner local variables
with ``scope="local"``. In addition, the ``-A`` option of ``simplifygnx`` is
used to perform a very simple-minded allocation of local variables to 
positions in the stack-frame using the ``slot=NUMBER`` attribute.

Global Variables
~~~~~~~~~~~~~~~~
A global variable is uniquely defined by both package and name so in addition
reference elements have the pattern:

.. code-block:: xml

    <id name=NAME def.pkg=PACKAGE scope="global"/>

The compiler simply pushes references onto the stack using the  ``push.global`` 
instruction.

.. code-block:: xml

    <push.global name=NAME def.pkg=PACKAGE/>


Local Variables
~~~~~~~~~~~~~~~
A local variable is defined in GingerXML as follows. 

.. code-block:: xml

    <id name=NAME scope="local" slot=SLOT_NUMBER/>

The compiler simply pushes references onto the stack using the  ``push.local`` 
instruction. 

.. code-block:: xml

    <push.local local=SLOT_NUMBER/>

Note that the ``slot`` attribute field is generated by using the option ``-A`` 
on the ``simplifygnx`` tool. This is simplistic algorithm and the compiler tool
is free to improve on it. It probably makes sense to start
using it with the intention of replacing it.


Sequence
--------
Sequences simply chain together expressions in Ginger. Because Ginger supports
multiple-values, compiling a sequence is as simple as concatenating the 
compiled instructions for each expression in turn.

.. code-block:: xml

    <seq> EXPR1 EXPR2 ... EXPRn </seq>

For the convenience of delivering a single result, InstructionXML allows a 
sequence of instructions to be bundled up as a ``seq``. These are automatically
flattened by the Ginger Runtime. You are not obliged to use these.

.. code-block:: xml

    <seq>
        instructions( EXPR1 )
        instructions( EXPR2 )
        ... 
        instructions( EXPRn )
    </seq>

System Function Applications
----------------------------
System functions are built-in to the Ginger Runtime, each with a unique
name. Calling them is especially efficient. 

.. code-block:: xml

    <sysapp name=NAME> EXPR1 EXPR2 ... EXPRn </sysapp>

Arguments are passed on the stack but, because Ginger allows multiple valued 
expressions, the count of the argument has to be computed and placed into 
VMCOUNT. A typical way to compile this would be as follows, using ``start.mark``
and ``set.count.mark``

.. code-block:: xml


    <seq>
        <!-- Put the stacklength in the slot NUM -->
        <start.mark local=NUM/>
        <seq>
            <!-- Compile the arguments -->
            instructions( EXPR1 )
            instructions( EXPR2 )
            ... 
            instructions( EXPRn )
        </set>
        <!-- Find the difference between stacklength now and the value in NUM -->
        <!-- and put the difference in the virtual register VMCOUNT -->
        <set.count.mark local=NUM/>
        <!-- Finally invoke the system-function -->
        <syscall name=SYSFN_NAME/>
    <seq/>

It may be possible to statically compute the number of arguments the 
sub-expressions will have. In that case there is a more efficient
ways to invoke a syscall. For example if we know that there are exactly 
N arguments, we should use ``set.count.syscall``.

.. code-block:: xml

    <seq>
        <seq>
            <!-- Compile the arguments -->
            instructions( EXPR1 )
            instructions( EXPR2 )
            ... 
            instructions( EXPRn )
        </set>
        <!-- Call with N arguments -->
        <set.count.syscall name=SYSFN_NAME count=N />
    <seq/>



Function Application
--------------------
Programmer defined functions are invoked through the ``app`` element.
This has exactly two arguments: a function to invoke and the arguments
to pass to the invocation.

.. code-block:: xml

    <app> FN_EXPR ARG_EXPR </app>

At the virtual-machine level, the VMCOUNT register must be set with
the number of arguments being passed across. In addition, the function
argument is restricted to evaluating to a single result.

Calling functions is very common and important, so the Ginger Runtime
compiler tries to use specialised instructions where it can. For
example, it uses the *arity* attributes to avoid generating
run time checks on the FN_EXPR in many common situations and to
avoid the necessity of dynamically calculating the number of arguments
being passed.

However these overheads cannot always be avoided. As a consequence the 
general function call looks like this:

.. code-block:: xml

    <seq>
        <!-- Compute the arguments -->
        <start.mark local=TMP0 />
        instructions( ARG_EXPR )
        <!- Compute the single valued function -->
        <start.mark local=TMP1 />
        instructions( FN_EXPR )
        <check.mark1 />
        <!-- Now call the function that is on the stack -->
        <end1.calls local=TMP0 />
    <seq/>

More frequently the compiler knows that FN_EXPR yields a single value 
in which case the following code is slightly better.

.. code-block:: xml

    <seq>
        <!-- Compute the arguments -->
        <start.mark local=TMP0 />
        instructions( ARG_EXPR )
        <!- Compute the single valued function -->
        instructions( FN_EXPR )
        <!-- Now call the function that is on the stack -->
        <end1.calls local=TMP0 />
    <seq/>

And more frequently, the compiler also knows the number of arguments that
ARG_EXPR would push. In that case it can be simplified yet further.

.. code-block:: xml

    <seq>
        <!-- Compute the arguments -->
        instructions( ARG_EXPR )
        <!-- Compute the funcion (on the stack) -->
        instructions( FN_EXPR )
        <!-- Now call the function that is on the stack -->
        <set.count.calls count=NUM_ARGS />
    <seq/>

And a very common case indeed is that the function being called is held 
in a global variable, in which case the ``set.count.call.global`` instruction
is used.

.. code-block:: xml

    <seq>
        <!-- Compute the arguments -->
        instructions( ARG_EXPR )
        <!-- Call the named global function -->
        <set.count.call.global def.pkg=PKG name=NAME count=NUM_ARGS />
    <seq/>



Assignment
----------
Assignments in GingerXML are defined in 'reverse' order; the not-very strong 
logic behind this convention is that the source value is computed before the 
destination. 

.. code-block:: xml

    <set> SRC_EXPR DEST_EXPR </set>

There are two main cases, assignment to a variable and assignment
to a function-call like expression. But each of these breaks down into
sub-cases.

Assignment to Local Variable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
When DEST_EXPR is a local variable ``<id name=NAME scope="local" slot=SLOT />``
the Ginger Runtime uses the ``pop.local`` instruction. It also ensures that 
the SRC_EXPR delivers one and only on result - either through arity analysis
or by using ``start.mark`` and ``check.mark1``. The latter is illustrated below.

.. code-block:: xml

    <seq>
        <start.mark local=TMP0 />
        instructions( SRC_EXPR )
        <check.mark1 local=TMP0 />
        <pop.local local=SLOT />
    </seq>

Assignment to Global Variable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is similar to the local case except that the ``pop.global`` instruction is
used. So for ``<id scope="global" name=NAME def.pkg=PKG />``:

.. code-block:: xml

    <seq>
        <start.mark local=TMP0 />
        instructions( SRC_EXPR )
        <check.mark1 local=TMP0 />
        <pop.global name=NAME def.pkg=PKG />
    </seq>

Assignment to a Sequence
~~~~~~~~~~~~~~~~~~~~~~~~
Ginger allows assignments to several variables in a row, such as 

.. code-block:: text

    ( 99, 88 ) -> ( x, y );

This simply generates a series of ``pop.local`` and ``pop.global`` 
expressions as appropriate.


Assignment to Other Expressions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is an area that has not been implemented yet in the current code-base
but is planned as to how it should work. 

Assignments to ``<if/>`` elements should compile the SRC_EXPR but
use the predicate of the ``if`` to select the DST_EXPR.

Assignments to ``<app/>`` elements should be translated from
``<set> SRC_EXPR <app> FN_EXPR ARG_EXPR </app> </set>`` into the
below. The SRC_EXPR and ARG_EXPR values are simply passed across
to the *updater* of the function.

.. code-block:: xml

    <app> 
        <sysapp name="updater"/> FN_EXPR </sysapp> 
        <seq> SRC_EXPR ARG_EXPR </seq> 
    </app>

Assignments to ``<sysapp/>`` elements will become:

.. code-block:: xml

    <sysupdate name=NAME>
        SRC_EXPR 
        ARG_EXPR
    </sysupdate>


Conditional Expressions
-----------------------
Conditional expressions are formed using the ``if`` element. This has
zero (!) or more (!!) arguments. If there are an odd number of arguments then
the last argument is an *else* clause. Otherwise the arguments pair up into
*guard* and *action* pairs. 

The rather peculiar ``<if/>`` elements is therefore equivalent to do-nothing
or ``<seq>``. The equally peculiar ``<if> EXPR </if>`` is equivalent to 
``EXPR``. Simplifygnx should (but doesn't right now) eliminate these oddballs.

To keep it simple, we'll look at the case with 2-arguments (if-then) and the
case with 3-arguments (if-then-else). 

If-Then
~~~~~~~
Guards are required to evaluate to a single boolean value. 

.. code-block:: xml

    <if name=NAME>
        GUARD_EXPR0
        ACTION_EXPR0
    </if>

Compilation relies on the ``ifnot`` instruction. There is a corresponding 
``ifso`` instruction for dealing with negated conditions. The distance jumped 
forward is the sum of widths of the ACTION_EXPR0 instructions (plus 1).

.. code-block:: xml

    <seq>
        <start.mark local=TMP0/>
        instructions( GUARD_EXPR0 )
        <check.mark1 local=TMP0/>
        <ifnot to=TBC to.label="done" />
        instructions( ACTION_EXPR0 )
        <!-- seq used to mimick a non-op -->
        <seq label="done"/>     
    </seq>

If-Then-Else
~~~~~~~~~~~~

.. code-block:: xml

    <if name=NAME>
        GUARD_EXPR0
        ACTION_EXPR0
        ELSE_EXPR
    </if>

The simple-minded approach is to generate a ``goto`` with the target being the
end of the loop. 

.. code-block:: xml

    <seq>
        <start.mark local=TMP0/>
        instructions( GUARD_EXPR0 )
        <check.mark1 local=TMP0/>
        <ifnot to=TBC to.label="else" />
        instructions( ACTION_EXPR0 )
        <goto to=TBC to.label="done" />
        <seq label="else" />
        instructions( ACTION_EXPR0 )
        <seq label="done" />     
    </seq>

The problem with generating code this way is that it is likely to generate
jumps-to-jumps. The trick to avoid this is to pass a 'continuation' label 
into the function that compiles an expression. The idea is that compiling
an expression includes the transfer of control.

Sketching this in (say) Python3, it would look like this. Note that this
sketch assumes that the final calculation of jump-distances is handled
in a later phase.

.. code-block:: python

    def compileExpression( expr, contn_label ):
        # List of instructions we will add to.
        instructions = MinXML( "seq" )

        if expr.getName() == "constant":
            instructions.add( MinXML( "pushq", expr ) )
            simpleContinuation( contn_label )
        
        elif expr.getName() == "and":
            # First expression must carry on in this sequence
            # so we pass the fake label Label.CONTINUE.
            lhs = compileSingleValue( expr[0], Label.CONTINUE );
            instructions.add( lhs )

            # If false jump to the label immediately.
            iand = MinXML( "and", to_label=contn_label )
            instructions.add( iand )

            # Run the rhs & continue to the label.
            rhs = compileExpression( expr[1], contn_label ) 
        
        elif .... :
            ....
            ....
        
        return instructions

    def simpleContinuation( contn_label ):
        '''Compiles an explicit jump to the label'''
        if label == Label.CONTINUE:
            return MinXML( "seq" )
        elif label == Label.RETURN:
            return MinXML( "return" )
        else:
            return MinXML( "goto", to_label=contn_label )


List Expressions
----------------
Immutable, singly linked lists are constructed via the ``[% ... %]`` syntax.
The GingerXML that corresponds to this is:

.. code-block:: xml

    <list>
        EXPR1
        EXPR2
        ...
        EXPRn
    </list>

This gets translated in the obvious way into:

.. code-block:: xml

    <seq>
        <start.mark local=TMP />
        instructions( EXPR1 )
        instructions( EXPR2 )
        ...
        instructions( EXPRn )
        <set.count.mark local=TMP />
        <syscall name="newList" />
    </seq>


Vector Expressions
------------------
An immutable 1D array is call a vector and is constructed via the ``[ ... ]`` 
syntax. The GingerXML this corresponds to is:

.. code-block:: xml

    <vector>
        EXPR1
        EXPR2
        ...
        EXPRn
    </vector>

This gets translated in the obvious way into:

.. code-block:: xml

    <seq>
        <start.mark local=TMP />
        instructions( EXPR1 )
        instructions( EXPR2 )
        ...
        instructions( EXPRn )
        <set.count.mark local=TMP />
        <syscall name="newVector" />
    </seq>


