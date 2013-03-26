Notes from a Conversation with GJH on the Ginger Garbage Collector
==================================================================

So if we sas "Ginger has a garbage collector" there should be a page which 
explains what a garbage collector is and why the Ginger garbage collector 
is special. Here's some reasons:

Feature: The Ginger garbage collection is unexpectedly lightweight 
because the Ginger VM uses garbage collector friendly structures. For example: 
here's the Ginger heap when ginger starts up ....

	(As revealed by gvmtest ….)
	rlwrap -S '%% ' -p Green ./gvmtest
	%% <heap.crawl/>
	Start Cage
	   Serial : 0
	   Used   : 0
	   Capacity: 1048576
	End Cage

What this tells us is that there's only one "cage" allocated for heap objects 
at the start of the GVM and it starts completely empty - because we have lots
of primitive types that don't need to appear in the heap in the first place.

And we don't just support records and arrays. There's another kind of heap 
object which is a mixed record/vector. It's an object that has a bunch of 
record fields and a variable sized array.

These are really garbage collector friendly. For example, if you want to 
represent (say) a fancy vector without these you'd probably have designed it 
as a  record object with a pointer to a vector object.

Let's suppose you want to have a vector plus a hashcode, for example (So that 
you can do super-fast equality testing).

Instead of a record object with CLASS, HASHCODE, POINTER and a vector with 
LENGTH, CLASS, VALUE* fields you have a single object CLASS, HASHCODE, 
LENGTH, VALUE*

	* The first is 2 objects which consumes 5+N words
	* The second is 1 object which consumes 3 + N words

But there's an additional cost for having two objects which is that there's 
a per object overhead inside every garbage collector. So with mixed-records 
the object is smaller (which is always good) meaning less tracing and 
less copying and involves only one GC loop. It'll be roughly twice as 
efficient for small objects. (For large arrays the difference is negligible 
since all the time will be taken up with copying.)

Feature: The Ginger garbage collector has weak references and weak tables built 
into it.

Feature: (for version 0.8.3) Ginger closes operating system resources when they
are collected. And when Ginger tries to allocate an operating system resource 
and the request is refused, Ginger will schedule a special garbage collection 
to close unused resources.

Feature: it doesn't matter how many dead objects you have in the heap 
because the Ginger garbage collector won't spent time on them when it 
runs (that's because it is a modified Cheny style garbage collector)

And that means it is surprisingly cheap to allocate objects that you use once 
and then don't use again. It just means that Ginger will try to expand store 
to keep the frequency of garbage collections down.

In other words the more tightly you manage your store, the less space Ginger 
needs. But even if you turn store over like crazy all that happens is 
Ginger needs more room. The garbage collections take no extra time and
they don't even happen more frequently.


Feature: Symbols (atoms) don't live in the heap at all. Again, gvmtest \
exposes this ...

	steve% rlwrap -S '%% ' -p Green ./gvmtest
	%% <compile><constant type="symbol" value="hello world"/></compile>
	define: 0 args, 0 locals, 0 results, 0 #words used
	enddefine
	Note: Virtual machine stopped normally
	%% <gc/>
	Garbage collection time: 0.026ms
	Number of objects copied             : 0
	Number of function objects copied    : 0
	Number of vector objects copied      : 0
	Number of record objects copied      : 0
	Number of user objects copied        : 0
	Number of string objects copied      : 0
	Number of word-record objects copied : 0
	%% <stack/>
	`hello world`
	%% 

There's a symbol `hello world` on the stack … but no objects in the heap at all.
We don't put 'em in the heap - they are stored in a dictionary external to the 
heap - and garbage collected properly.

A key motivation in keeping symbols out of the heap is that the garbage 
collector works extremely well for ephemeral objects but long-lived objects 
incur a cost every time they are collected.

Symbols are, as a category, typically long-lived. So we keep them out of the
heap and manage them separately.

[Aside: Functions are the same. Which is why system functions, which every 
program is likely to make heavy use of, are not allocated in the heap at 
all.]

Notes
------
A planned revision to the garbage collector will allocated functions in a 
special "don't collect often" area.

Very large objects will be allocated in a special area ("cage") marked for 
non-copying. Each large object will be allocated its own cage. When the large 
object is freed by the garbage collector, the entire cage is freed.
