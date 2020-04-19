# Ginger On One Page - Common Syntax 'Cheatsheet'

This note provides an at-a-glance summary of the common-syntax and indirectly 
summarises the Ginger language itself.

## Comments

* All non-functional code is introduced with a hash. 
* End of line comment introduced with:
    - `#` followed by anything except punctuation, which means something different
* Processing instructions introduce with sequence that acts like an end-of-line
  comment
    - `/#!/`
* Nesting long comment introduced with 1 hashes and a bracket, closed with 
  hash and the complementary bracket
    - `/#[<>(){}\[\]]/`

## Literals (Constants)

### Absent aka null
* Absent is an escape type.
* Absent: absent, null is a synonym

### Boolean literals
* Boolean: false, true

### Number Literals
* Integer: ..., -1, 0, 1, 2 ... (unlimited precision)
    - Also hex `/0x[_0-9A-F]+/` TODO!
    - binary `/0b[_01]+/` TODO!
    - radix `/[1-3]?[0-9]r[_0-9A-Z]+/` TODO!
* Floating Point: usual notation, double precision

### String, Character and Symbol Literals
* String: Single or double quotes with backslash escape, incl interpolation
    - `"This is a string\n"`
    - `'My name is \\(name)'` - interpolation
    - (Planned) Unicode strings are natively supported.

* Long string aka text block: Tripled single/double-quotes, acts as a long-string with ordinary escapes suppressed. A triple backslash can be used to ignore leading whitespace and hence establish a left-margin.
```
# Long string on one line - useful for ignoring escapes etc. It may not
# contains newlines.
"""....."""

# More commmonly - long string across multiple lines. The opening
# """ must be followed by a newline and it determines the indentation level.
# The closing triple-quotes may not be indented less than the opening quotes.
# Typically they are on a line of their own at the same level as the opening
# quotes.
"""
....
....
"""
```

* Character: string literal followed by triple-dots i.e. `...`. This is guaranteed to expand into a sequence of characters.

* Words/Symbol/Selector/Atom/Tag: quoted with backticks. e.g. ``` `a` ```

### Regular Expression literals

* Regex `///...///`
    - e.g. `///[^/]+/[^/]*\.gif///`
    - `if ///[a-z]*/// @matches x then ... endif` TODO!


## Collections With Dedicated Syntax

The collection 'caching' constructors have two special properties:

1. They always return immutable values. If you want mutable values then you should use an explicit mutable constructor or use `toFluid` to convert to a fully modifiable value or `toRigid` to one that supports replacement of members (values).

2.  They may return a new instance or reuse a previously constructed instance. In effect they reserve the right to cache any value that was previously constructed with identical values i.e. they compare as shallow-equals. If you want a guaranteed unique values you should use an explicit non-caching constructor or simply copy the result.

3. At compile-time, an expression that consists only of literals and cached-collections is guaranteed to be constructed only once. The bracket notation is provided to signal those consructors for which this guarantee is made.

### Positionally Indexed

| Brackets   | Constructor | Data-type              |
| :--------- | :---------- | :--------------------- |
| [! x, y !] | Tuple       | tuple (long form only) |
| [ x, y ]   | Vector      | vector (short)         |
| [% x, y %] | List        | list (long form only)  |
| [: x, y :] | Stream      | stream (long only)     |


#### Mnemonics
* ! = exclamation is made from stroke+dot, which are different kinds of mark, like the different types in a tuple.
* % = two things linked by a slash, for a linked list
* : = one-dot-after-another, like a stream


### Key-Indexed

| Brackets           | Constructor | Data-type      |
| :----------------- | :---------- | :------------- |
| { a:x, b:y }       | Record      | record (short) |
| { a=x, b=y }       | Record      | record (long)  |
| { a => x, b => y } | Map         | map (short)    |
| {? x, y ?}         | Set         | set (long)     |


#### Mnemonics
* ? = are you in or out of the set?

## Finite Collections (including those with dedicated syntax)

Ginger provides a wide range of built-in collections. These are each equipped with specialised implementations for Immutable, Updateable and Dynamic versions: Immutable implementations cannot be modified, Updateable implementations can have members updated but not added or deleted, but Dynamic implementations support all operations. Dynamic implementations support the irreversible `freeze` operation that prevents further updates
and authorises the garbage collector to replace the object with an optimised Immutable implementation.


| Type   | Brackets        | Constructors                           | Notes                                   |
| :----- | :-------------- | :------------------------------------- | :-------------------------------------- |
| Bag    |                 | Bag UpdateableBag DynamicBag           | Bags may support 0 or even -ve values   |
| List   | [% ... %]       | List                                   | Linked lists are immutable              |
| Map    | { a => b, ... } | Map UpdateableMap DynamicMap           |                                         |
| Record | { a: b, ... }   | Record UpdateableRecord                | Records cannot support adding/deleting. |
| Set    | {? ... ?}       | Set DynamicSet                         | Sets cannot support updating.           |
| Tuple  | [! ... !]       | Tuple UpdateableTuple                  | Tuples cannot support adding/deleting   |
| Vector | [ ... ]         | Vector UpdateableVector DynamicVector  |                                         |


## Series and Streams

In addition to finite collections, Ginger provides support for indefinitely long sequences of values called _series_. Series might have a definite length or might not and, as a consequence, it is not entirely safe to take the length of a series. They can be populated from an explicitly supplied sequence of values, from the contents of a file, or procedurally generated from a function. For many practical purposes, you can think of series as lazy linked lists whose 'far end' is expanded on demand.

```
Series( 'cat', 'dog' 'cow' ) -> SERIES
SeriesFromFunction( INITIAL_STATE, TRANSFORMER ) -> SERIES
    where TRANSFORMER( STATE0 ) -> ( VALUE, STATE1 )
```

Importantly, series efficiently support the `head`, `tail` and `skip` operators and calling a series will return the head and tail (or escape with `Stop`). This makes them closely related to _streams_. A stream represents a cursor into a series and each time it is called returns the current value (head) and advances the cursor along the series (updates to the tail). If the series is empty then the attempt to get the next value will escape with `Stop`.

Streams and Series can be freely interconverted:

```
STREAM.to( Series ) -> SERIES
SERIES.to( Stream ) -> STREAM
```

### Sharing Between Series and Streams

Streams are imperative and support all kinds of alteration including `pushBack` and `replaceItem`. However Series are completely immutable and updating a Stream that is built from a Series will have no effect on that Series. However, a Series that is built from a Stream will be dynamically expanded each time the Stream is called and will call the stream if it is forced to expand.

## Variables

### Syntax

* Normally unquoted that does not clash with reserved words: 
    - [_a-zA-Z][_a-zA-Z0-9]* e.g. fred99
* But may be escaped to avoid reserved words clash or to use special characters.
    - \if                                                                  TODO!
    - \"what the heck"

### Simple Bindings
TBD

### Assignment to a Variable

* EXPR -> VAR, VAR <- EXPR
* VAR ::= EXPR, EXPR =:: VAR

## Environment Variables

* ${ENVVAR} reference, if ENVVAR does not exist this will escape (Absent).
* $?{ENVVAR} reference or absent
* ${ENVVAR} ::= <EXPR> assignment




## Parenthesized Expressions
* ( E ) just means E but determines the precedence.


## Function Calls
* Prefix form F( E1, E2, ... )
* First infix form E1 .F E2
* Second infix form E1 @F E2 e.g.

* F?( E1, E2, ... )
* E1 .?F E2
* E1 @?F E2

QUESTION: What about named parameters? 

### Partial Application

* F(% 1, ?, ? %)
* var double := (% ?1 + ?1 %)



### Arithmetic Operators

* E + E
* E - E
* E * E
* E / E       <- floating point result
* E */ E      <- use this for rational division
* E .div E
* E .mod E
* E .divMod E

Notes: `div` is integer division truncated toward negative infinity
mod is integer modulus, satisfying:
```
(x .div y)*y + (x .mod y) == x
```

Reason: 
  We take the meaning of a negative modulus to be same as the
  positive modulus but presented using negative values.

  e.g. in modulus 7 we have [ 0, 1, 2, 3, 4, 5, 6 ]
  which is also             [ 0, -6, -5, -4, -3, -2, -1 ]

This means that the result is signed consistently with the
denominator.

### Bitwise Operators

* E .bitAnd E
* E .bitOr E
* E .bitNot
* E .bitNor E
* E .bitNand E
* E .bitEq E
* E .bitNeq E
* E .bitXor E

N.B. The Ginger bitwise operator do *not* work on integral values directly, but should be converted into bit-patterns. Bit-patterns and integral values can be freely inter-converted but the programmer is obliged to make the width of the bit-pattern explicit (and may be infinite).

### Boolean Operators
* E and E
* E or E
* not( E )

### Escape-oriented Operators

* E && E    consider E AND E    E and? E
* E || E    consider E OR E     E or? E
* !! E      consider NOT( E )   not?( E ) <- problem f?( x ) should probably be re-escape.
* ?? E

### Relational Operators
* Relational operator form, allows chaining e.g. E1 < E2 < E3
    - Relational operators are: < <= == >= > != =^= !=^=
    - Chaining                                                             TODO!

### Sequence Operators

* Concatenation & function composition EXPR ++ EXPR
* Explode operator ``...``.



## Assignment to Function Calls

To be done

## Bindings

To be done

# Queries, Conditionals and Loops

### Queries

### Conditionals
Main question is what's the context between ``if`` and ``then``. Is it an 
expression context or a query context?


### Loops
To be done

## Exceptions: Escapes, Rollbacks, Failovers & Panics
To be done


## Function Definitions
To be done

## User Defined Classes
To be done

## Method Definitions
To be done
