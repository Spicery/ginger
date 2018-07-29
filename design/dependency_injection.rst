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




Notes
-----
* Good article by Martin Fowler - https://martinfowler.com/articles/injection.html