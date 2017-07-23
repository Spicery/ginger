The Design of Iterators in AppGinger
====================================

How does a loop such as 

	for i in EXPR do STMNTS endfor
	
get translated into an iterator? The issue is that you need completely different iterators depending on the type of EXPR. 

In order to avoid allocating store in the common cases, the low level interface splits up the components of the iterator into three parts. These are the 

	1. 	state <- the bit that varies
	2.	context <- the bit that is constant
	3.	next_fn <- the function that advances the iteration
	
The loop is compiled into code looking a bit like this.

	var ( state, context, next_fn ) = sysGetFastIterator( EXPR );
	for
	until
		var i;
		next_fn( state, context ) -> ( i, state );
		state == termin
	do
		STMNTS
	endfor
	
In other words, the termination of the loop is signalled when
the state variable is set to the special value <termin>. Obviously iterators should be designed so that <termin> is not accidently 
returned.

To see how this works out in practice, iteration over a list works by setting state to successive pairs of the list, setting context to a dummy value and next_fn to something like this.

	define list_next_fn( state, context ) =>
		if state.isPair then
			state.destPair
		else
			_, termin
		endif
	enddefine
	
In the case of a vector, the state is the index so far and the context is the vector itself. The next_fn looks something like this.

	define vector_next_fn( state, context ) =>
		if state <= context.length then
			context.index( state );
			state + 1 ->> state
		else
			_, termin
		endif
	enddefine
	
The system function sysGetFastIterator is not normally visible to a programmer and so is free to return fast-but-unsafe versions of these next_fns. In general a low-level iterator can rely on being passed two arguments state & context of the correct type, state never being <termin>.

