The Ginger Virtual Machine
==========================

The Ginger Virtual Machine is made up of the following main areas:

	* Virtual registers
	* Value stack
	* Call stack
	* Heap
	* Symbol dictionary

Each of these 

Cell
----
The fundamental value of the GVM is a fixed-width word, which is 
typically 32/64-bit depending on how the GVM is compiled. 

It is defined as a (void *) but frequently cast to a signed or unsigned 
integer and manipulated as a small bit-vector.

The 2 low-order bits of a cell are treated as a tag bit. By design, when these
two bits are 0x0, the cells represents a small-integer. Pointers have a 
tag of 0x1.

Virtual Registers
-----------------

	* PC 	virtual program counter, pointer to currently executing instruction
	* FUNC 	pointer to currently executing function
	* COUNT the number of items passed on the stack to the current call
	* LINK  the previous PC i.e. return address
	* FUNCLINK the previous value of FUNC

Due to the design of function objects, it is not straightforward to figure
out from the PC what the enclosing function object is. But the
garbage collector needs to know what function objects are in use! To compensate 
for this design error, we cache the currently executing function object.
[Aside: this has to be sorted out in the future as it introduces a 
superfluous save/restore penalty on every function call! The way to do it is to 
perform a forward instruction crawl from the PC until a special marker
instruction is found, followed by a pointer back to the key of the 
function object.]

The COUNT register is set by the caller, to inform the callee the effective
size of the value stack. The callee may not decrease the value stack below
that value. [Aside: this is not a hard restriction but one introduced for
the semantics of Ginger. If you wanted to repurpose the GVM for other languages
you might replace this with underflow checking (or guard pages).]

The COUNT register is not preserved but immediately obliterated by the next
function call.

The LINK and FUNCLINK registers are set by the caller when invoking a 
function object. It is up to the callee to save these values onto the call
stack if it intends to call sub-functions.

Note that core-functions are especially efficient and special forms of
call instruction allow them to bypass saving and restoring PC and LINK.

Value Stack
-----------
This is a simple stack of cell values which is pushed and popped. However
it is implicitly segmented by the callstack. A function is not permitted to
take more than COUNT items off the stack.

Callstack
---------
The callstack is a highly structured stack, segmented into frames that are
pushed and popped in their entirety. 

Heap
----
The heap is itself a collection of sub-heaps called "cages". The phrase
cage comes from the metaphor of the heap being a collection of animals where
animals of incompatible types are separately 'caged'.

Aside: At the time of writing, cages are private to a heap but the long term 
plan is to share cages of deeply-immutable objects, where deeply-immutable means 
that all other objects reachable from the chosen object are also immutable. The
garbage collector will be responsible for migrating objects into the shareable,
cages of deeply-immutable objects.

Symbols Dictionary
------------------
This is an area dedicated to the storage of symbols, which was an early 
decision to have a robust but simple implementation of symbols. (It's one of 
the many tricks used to keep non-ephemeral objects out of the heap.)
