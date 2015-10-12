Design of the fetchgnx Utility
==============================

Introduction
------------
The concept behind fetchgnx is to isolate the knowledge of the on-disk 
structure of Ginger projects (aka inventories). There are three tasks that
requires the knowledge of projects: 

  # resolving a reference to a global variable from an enclosing package,
  	* 1st case, resolve a qualified reference
  	* 2nd case, resolve an unqualified reference
  # fetching the definition of a global variable from a defining package,
  # fetching the on-load initialisation code for a package.

Arguably these are significantly separate functions. However because there
is inevitably a great deal of shared code, I allowed for the possibility of
a single shared executable in the design of the API.


API
---
The API was initially based around the use of command line parameters, shown
below::

	Usage:  fetchgnx MODE_OPTION -j PROJECT -p PACKAGE [-a ALIAS] -v VARIABLE
	MODE OPTIONS
	-R, --resolve         find the origin package::variable of a reference
	-D, --definition      find the definition of a package::variable
	-H, --help[=TOPIC]    help info on optional topic (see --help=help)
	-I, --initial         fetch initialisation code for a package
	-V, --version         print out version information and exit
	-L, --license[=PART]  print out license information and exit (see --help=license)
	ARGUMENTS FOR -R AND -D
	-j, --project=PATH    defines project folder, there may be more than one
	-f, --folder=PATH     alternative to --project option
	-p, --package=NAME    sets the package name
	-a, --alias=NAME      sets the alias name, optional
	-v, --variable=NAME   sets the variable name
	-u, --undefined       allow undefined variables

This quickly turned out to be clumsy. So an undocumented mode -X was
added that enabled different requests to be submitted as MinXML. The 
different requests are::

	<resolve.qualified pkg.name=${ENCPKG} alias.name=${ALIAS} var.name=${VAR} />
	<resolve.unqualified pkg.name=${ENCPKG} var.name=${VAR} />
	<fetch.definition pkg.name=${DEFPKG} var.name=${VAR} />
	<fetch.pkg.init pkg.name=${LOADPKG} />
	<fetch.load.file pkg.name=${LOADPKG} load.file=${FILE} />
	<browse.packages/>

At the time of writing, the system uses an awkward mixture of these two APIs.

Criticisms
----------

  * The command-line API is a burden to reimplement, so any alternative
    tool would be dominated by the command-line processing.

  * Supplying the projects via the command-line requires alternative
    implementations to handle both processing MinXML and the command line.
    It should be possible to supply the project path in the same MinXML.

  * No consideration was given to the possibility that there might be
    projects with radically different implementations in the same call. 
    This is a serious weakness as an early goal was to support SQL based
    projects and these would need to be mixed with file-based projects.

  * Resolving on an identifier-by-identifier basis is clearly inefficient.
    It would be much more attractive to supply the GNX and perform a 
    bulk resolution. To avoid the tool having to know the structure of GNX
    the variables-to-be-resolved would be clearly marked by another
    processing stage (simplifygnx).

Redesign
--------
The most important objective must be to support different formats for
projects. This can be handled by supplying a utility for each format that:-

  * Returns index information for the project in MinXML;
  * Returns definition-content for a variable from a defining package of a project;
  * Returns package initialisation from a package of a project.

The two formats would be the SQLITE3 inventory (*.ivy) format and the Ginger
project folder format (*.gproj). The utilties would be respectively.

  * ivytool [OPTIONS] <pathname>
  * gprojtool [OPTIONS] <pathname>

where OPTIONS are

  * -Q, --query
  * -H, --help
  * -L, --licence
  * -V, --version

The MinXML understood by both tools would be:

   * <fetch.index/>
   * <fetch.var.gnx def.pkg=${DEFPKG} var.name=${VAR} />
   * <fetch.init.gnx def.pkg=${DEFPKG} />

Fetching Index Information
~~~~~~~~~~~~~~~~~~~~~~~~~~

This should return MinXML that covers the set of defined packages, their names, 
their imports, and the set of defined variables and their tags.::

	INDEX ::= <project name=${PROJECT_NAME}> PACKAGE* </project>
	PACKAGE ::= <package name=${PACKAGE_NAME}> IMPORTS INCLUDES DEFINITIONS </package>
	IMPORTS ::= <imports> IMPORT* </imports>
	IMPORT ::= <import from.pkg=${PKG_NAME} [ alias=${ALIAS} ] from.tags=${TAGS} to.tags=${TAGS1} />
	INCLUDE ::= <include from.pkg=${PKG_NAME} [ alias=${ALIAS} ] from.tags=${TAGS} to.tags=${TAGS1}/>
	DEFINITIONS ::= <definitions> DEFINITON* </definitions>
	DEFINITION ::= <define var.name=${VAR} tags=${TAGS} />

For example::

	<project name="Foo">
		<package name="maths=">
			<imports>
				<import from.pkg="ginger.constants" from.tags="public" to.tags="public" />
			</imports>
			<definitions>
				<define var.name="pi" tags="public" />
				<define var.name="e" tags="public" />
			</definitions>
		</package>
	</project>

Fetching GNX Code
~~~~~~~~~~~~~~~~~
Returns GNX code.

Fetching Package Initialisation GNX Code
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Returns GNX code.
