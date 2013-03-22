C++ Namespaces for Ginger
=========================
Stephen Leach, March 2013

The Ginger project began some 15 years ago as an extended hack while I was
on holiday in Dorset. It was written in C with a few modest C++ additions,w
which didn't include namespaces.

Now Ginger has proven to be much more long-lived I am slowly migrating
the names out from the default namespace. My current plan is to have
the following two namespaces:

	* Ginger - used for the public API. 
	* Ginger::DevTeam - Only available to the dev-team members.

The migration strategy is (1) to move all the code into the Ginger namespace
and (2) then to further migrate internals into the DevTeam subspace.