Double-Hat Operator
===================

Excerpted from a letter to Philip Allen
---------------------------------------

Another use-case that constantly crops up in debugging is wanting the name-and-value of a variable. Rightly or wrongly we have all found ourselves writing code like the following and becoming exasperated at the keyboard stutter.

println( "x = ", x, ", y = ", y, ", = z" );

In Ginger one would typically write this using the form ^^x that returns the maplet ( `x` => x ) as follows.

# The "with" operator supplies optional arguments.
println( ^^x, ^^y, ^^z ) with separator = ", ";
