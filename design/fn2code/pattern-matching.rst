%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Pattern Matching in Ginger
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Basics
------

To start with, consider the innocuous expression::

	x := 99

The := symbol is a pattern match, with the LHS being the pattern and the RHS being a source of values to match against. Because the LHS is a pattern, the variable “x” should be understood to be match-variable. Matching is the one-and-only way to declare variables in Ginger. (Yes, it looks like a normal language but under-the-hood not so much.)

So it is perfectly OK to write::

	99 := x

which means pattern-match 99 against the value of x. If x isn’t 99 the Ginger runtime makes a complaint. And it is OK to write::

	( x, y ) := ( y, x ) 

which means bind the variables x and y to the values of y and x, the latter being evaluated before the match. So that’s the idiomatic way to swap variables. The brackets have no semantics (not a tuple like Python3) but just makes it more readable. Ginger supports multiple values pervasively, so (y,x) is an expression that returns two values. 

And it is also OK to write::

	[x, y] := z

which means that z should be a 2-element vector that is unpacked and the first element put in x and the second in y. This works because it is syntactic sugar for newList( x, y ) := z and newList has an inverse. The compiler should translate this into::

	(x, y) := inverse( newList )( z )

All of the above are relatively common features in modern programming language
design. So now we move onto more unusual aspects.

Pattern-Matching Contexts (Actually Query Contexts)
---------------------------------------------------
There are various contexts in which pattern-matching get used. In fact, because pattern-matching is just one type of QUERY (more on that later), these are different query-contexts.

When you use a QUERY as an ordinary expression such as ``x := 99``, it _must_ succeed. It is effectively an assert. As it cannot fail, that's fine. But ``99 := x`` as a naked QUERY will escape unless the binding succeeds. Note that this is the context that function calls use to bind their input parameters.

But you can use a QUERY inside an IF as well. This has much the same effect as an "if let" in Swift. For example, in the example below the contents of the if-statement are only executed if f() returns exactly one value::

	if x := f() then
		... statements with 'x' bound ...
	endif

A pattern-match inside an IF statement does not have to match. If it does match then the 'then' part is executed with the bindings in scope. If it fails then the else part is executed without the match variables being visible.

Finally you can use a QUERY inside a FOR loop as well. In this case we find all the possible matches. So far we have no way of generating multiple matches - we'll get there. Therefore, right now, this isn't terribly interesting::

	for x := y do
		... executes once with x bound to the value of y ...
	endfor

Pattern-Matching Multiple Values (Variadic)
-------------------------------------------
The syntax for matching multiple values probably needs some TLC. But the concept is very powerful. The postfix operator '...' means return all-the-values-of and is commonly called the 'explode' operator. It has an inverse that aggregates all the values together. Using the explode operator we can match the head and tail of a vector::

	>>> [ x, y... ] := [ for i from 1 to 5 do i endfor ]
	>>> showMe( x, y )
	[1],[2,3,4,5]

Note that the compiler has to help a little here - how does it know that ``y`` should be a vector? The answer is that the compiler breaks the match down like this::

	[ x, y... ] := [ for i from 1 to 5 do i endfor ]
	tmp := [ for i from 1 to 5 do i endfor ] && x := tmp[1] && y := tmp[1:]

In programming languages that support variadic matching, such as Python3 or Scheme, there is usually a restriction that there's only one variadic parameter and it has a fixed position. There is no such restriction in Ginger. 

By having two or more variadic matches, it is possible to generate multiple solutions to a pattern-match::

	>>> for x..., y... := "bozo"... do showMe( x, y ) endfor
	"bozo","" 
	"boz","o"
	"bo","zo"
	"b","ozo"
	"","bozo"

There are a lot of subtle issues in the way pattern matching works. In the above example, the compiler is expected to bind ``x`` and ``y`` to strings - but that requires a specific untangling of the pattern-match to make sure that's true.

Getting Weirder - Assignments
-----------------------------
We're already a fair way off the beaten track of the majority of programming languages. Bearing in mind that variable binding is completely fundamental to getting anything done, the fact that Ginger is so complicated is something of a design risk. But there's more.

However, Ginger unifies assignment into the pattern match model. To make this understandable, it helps to introduce the following explicit _mode_ prefixes (syntax not finalised). These can be used to decorate any variable in the target of a pattern or an assignment.

  * ?x - x is a pattern variable
  * !x - x is an assignment variable
  * =x - x is a constant (evaluated and then stored)

So the following statement means - bind to the variable ``p`` to 77, assign 88 to ``q`` provided that the value of ``r`` is 99::

	( ?p, !q, =r ) := ( 77, 88, 99 )

Now assignments are potentially a bit tricky as they can 'spy' or even corrupt the order the match happens, which is very undesirable. The semantics of assignment is that they happen atomically when the match succeeds. 

Aside: when you get to take a look at the 'do' operator, you'll see that it is translated into a QUERY like this, with the assignments being pushed out as late as possible::

	( ( ?p, ?tmp_q, ?tmp_r ) := ( 77, 88, 99 ) && r == tmp_r ) do q <- tmp_q

The crucial property is that if a match fails, it is as if no assignment took place. If you loop over a match with assignments, the assignments happen at each time through the loop but the last failure of the match, which terminates the loop, has no effect.

Even weirder, you can run the updater of a function this way too. In this trivial and somewhat stupid example we use pattern matching to run the updater of f::

	!f() := 66	# Same as f() <- 66

So - what is the difference between the assignment operator '<-' and the bind operator ':='? The answer is that they only differ in the _modes_ they apply by default. The following expressions mean the same::

	x, y... <- E 
	!x, !y... := E

Note that both the assignment and pattern match operators have reverse versions, with no difference in meaning. So if you see the following, there's nothing new::

	E -> x, y... 
	E =: !x, !y...	


Baby Steps to Logic Programming
-------------------------------
Now we take the crucial design step into logic programming. In logic programming we have a new concept, which is that of shared match values::

	if ?x, ?x := p, q then 
		... execute this code if p==q with x bound to p ...
	endif

Critically two unbound variables can be bound together without being bound to a value, creating a long-term constraint. For this we need the unification operator ``:=:`` - the colons at both ends are intended to be mnemonic that there are patterns at both ends. The following QUERY would constrain both x and y to be zero::

	?x :=: ?y && x := 0

The rest of logic programming only makes sense in the wider context of QUERYs and RULEs. And I think that's outside of the scope of this intro.
