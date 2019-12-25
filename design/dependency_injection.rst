%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Dependency Injection for Ginger
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

Objective
---------

Useful classes (or libraries) often require lots of implementations of
basic services in order to do their job. We say that such a class is
needy and has dependencies. We want to be able to write the needy class 
so that:

* The dependencies are satisfied by any class that meets an interface.
* The code does not include or even refer to an explicit implementation of
a dependency - although we are prepared to identify dependencies.
* We do not want to create lots of plumbing that tediously passes instances 
dependencies.
* Work by independent development teams naturally glue together.

Issues
------
							Currently			Considering
							---------			-----------
Symbol quotes				`foo` 				
Selectors										@foo /foo `foo \foo §foo
												nickname::`foo
Mapping selector to value	^^bar
Mapping variable to resolved name
												^^^foo
Package nickname separator	lists::isEmpty		
Type assertion				x : String
Optional parameters 		f( order=`asc` )
Annotations					[ order(`asc`) ]	[ order=/asc ]



var x := /foo;
/uk/org/spicery/myppkg/newMovieLister
[ `uk.org.spicery.myppkg`, `newMovieLister`]


def new( function, parameter_selector, target_type ) =>>
	lookup( function, parameter_selector ) ||
	lookup( function, target_type ) ||
	lookup( target_type )
end

def set( key ) =>>
	value -> lookup[ key ]
end




def newMovieLister( dep finder : MovieFinder ) =>>
	...
end

def newMovieLister( dep finder : MoveFinger ) =>>
	let finder := new( self, @finder, MovieFinder )
	in
		...
	end
end


[ constructorFor=MovieFinder ]
def newCSVMovieFinder( dep filename isa String ) =>>
	...
end


[ function=newCSVMoveFinder, parameter=@filename ]
def nameOfCSVFile() =>>
	"movies.csv"
end

def newCSVMovieFinder() =>>
	let filename := new( ::/^^/filename, ::/String )
	in
		...
	end
end


Notes
-----
* Good article by Martin Fowler - https://martinfowler.com/articles/injection.html