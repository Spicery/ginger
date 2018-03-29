%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Break Mode
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Living Without Semi-Colons
--------------------------
Do we need semi-colons in the Common syntax? Scripting languages like Python and Ruby have managed to banish the need for statement separators - could Ginger's Common syntax follow suit? An experimental ``--break`` option has been added to the parser common2gnx that uses a very simple algorithm to implement this.

	#. If the next token must be a semi-colon, but isn't
	#. And the --break option is enabled
	#. And the next token starts a new line
	#. Be happy

What About Parentheses & Brackets?
----------------------------------
There is one awkward ambiguity associated with ``(`` and ``[`` - these are keywords that can start an expression (prefix role) or occur in the middle of an expression (infix role). This means that, for example, the expression

.. code::

	x
	[]

would be treated as ``x[]`` and not as the more likely

.. code::

    x; []


The solution is to disable the infix role of ``(`` and ``[`` at the start of a line. So when these occur at the start of a line in ``--break`` mode, they are guaranteed to be used in their prefix capacity of grouping and list building respectively.

This does interfere with some indentation styles that @/Chris Dollin has used. But something had to give somewhere and this seemed like the least impact.

Compromises
-----------
Finally, this feature cannot be used interactively because it requires peeking ahead to see if the next line starts with an infix operator, which continues the current expression. 

Python doesn't have a problem with the terminal mode, so why does Ginger? The trick that Python uses to get around this depends on brackets being part of the world-of-expressions and not world-of-statements. That allows for brackets to have the role of disabling break-mode. 

But this comes at a steep price. This expression/statement dichotomy is a design disaster for Python that causes knock-on issues such as ``if`` and ``for`` having two different syntaxes in the different worlds and the inability to try/except an expression. That isn't a good compromise for any language and in Ginger it would be terrible.

Future
------
So at the terminal, the semi-colon plays the role of "send it now". In our future vision for Ginger, we would move away from the terminal for interactive exploration, retaining it only for legacy. Instead we see the "Ginger Terminal" as being more like a chat-application, where your commands are responded to by multiple "bots".