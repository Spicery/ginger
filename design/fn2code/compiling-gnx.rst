%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Compiling GingerXML into GingerVM-Code
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Introduction
============
The Ginger virtual machine is word-coded and stack-based. 

Word-coded means that an instruction consists of an instruction word 
followed by one or more data words, where each word has the same width 
as a C++ (void *). Although this isn't at all compact, it allows us
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

.. code-block:: XML

	<fn.code args.count="1" locals.count="1">
		... instructions ...
	</fn.code>

The first instruction we generate will be <enter1/>. This is a very specialised
instruction that builds a stack-frame for functions with one input and pops
the arguments off the stack into the frame. It must be 
the very first instruction as it uses offsets of the virtual-program-counter 
to find the locals-count. So our generated code will look like ...

... code-block:: XML

	<fn.code args.count="1" locals.count="1">
		<enter1/>
		... further instructions ...
	</fn.code>

The comparison x > 0 is tackled first. The simplest way to compile this is
with the stack-based code: push x, push 0, call greater than or equal to.

... code-block:: XML

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

... code-block:: XML

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

... code-block:: XML

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

.. code-block:: XML

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

..code-block:: XML
	<id name=NAME def.pkg=PACKAGE scope="global"/>

The compiler simply pushes references onto the stack using the  ``push.global`` 
instruction.

..code-block:: XML
	<push.global name=NAME def.pkg=PACKAGE/>


Local Variables
~~~~~~~~~~~~~~~
A local variable is defined in GingerXML as follows. 

..code-block:: XML
	<id name=NAME scope="local" slot=SLOT_NUMBER/>

The compiler simply pushes references onto the stack using the  ``push.local`` 
instruction. 

..code-block:: XML
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

..code-block:: XML
	<seq> EXPR1 EXPR2 ... EXPRn </seq>

For the convenience of delivering a single result, InstructionXML allows a 
sequence of instructions to be bundled up as a ``seq``. These are automatically
flattened by the Ginger Runtime.

..code-block:: XML
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

.. code-block:: XML
    <sysapp name=NAME> EXPR1 EXPR2 ... EXPRn </sysapp>

Arguments are passed on the stack but, because Ginger allows multiple valued 
expressions, the count of the argument has to be computed and placed into 
VMCOUNT. A typical way to compile this would be as follows, using ``start.mark``
and ``set.count.mark``

.. code-block:: XML
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

.. code-block:: XML
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

..code-block:: XML
	<app> FN_EXPR ARG_EXPR </app>




Assignment
----------
Assignments in GingerXML are defined in 'reverse'
order; the logic behind this is that the source value is computed before

..code-block:: XML
	<set> SRC_EXPR DEST_EXPR </set>

Assignment to Local Variable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~


Assignment to Global Variable
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Assignment to Function Application
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


