Design of the Inventory Format
==============================

The idea is to represent the whole of a project inside a single file, called
an inventory (\*.ivy). The inventory will be regarded as the library format
by the run-time system. The development loop will automatically detect when
the inventory is out of date with regard to source files and rebuild it on
the fly.

The interactive environment will be modelled by a temporary inventory that
is automatically maintained.

Existing Structure
------------------

The current file-based structure is as follows::

    <project folder>
        <package folder>            (many)
            imports.gnx             (one)
            <tag>.auto              (many)
                *.<known-extn>      (many)
            <tag>.lib               (many)
                *.<known-extn>      (one)
            load                    (one)
                *.<known-extn>      (many)
            help                    (one)
                *.txt               (many)


The advantage of this format are that 

  * The project name is the same as the top level folder.
  * It represents a collection of packages, named in an obvious way.
  * The packages share a set of imports.
  * Variables are provided in separate files, which supports autoloading.

The disadvantage are that

  * Tagging is very clumsy - it's not really possible to apply multiple tags.
  * There's no obvious place for self-documentation.
  * The format is not performant.

These disavantages are so severe, it will be necessary to redesign the format
somewhat. I suggest that it would be better to put the tagging information 

Inventory Structure
-------------------

The inventory will be a SQLITE3 file. It will capture :-

  * The name of the inventory and any documentation that goes with it.
  * The filenames of the source files that were compiled to build it, in 
    order to automatically detect when it needs rebuilding.
  * The declaration of variables, along with their tags and other 
    meta-info i.e. package, variable name, tags.
  * The GNX code of the variable that has to be loaded to compute
    their value.
  * Documentation associated with the variables.
  * Initialisation (load-time) code for the packages and the inventory.
    This is potentially a security risk, unless we cleanly separate
    inspection from loading.
  * Named entry points for packages and the inventory.

Here's a suggested schema. Note that the intention is to represent the initialisation code and entrypoints for the inventory via a fake package name, which would be the empty string::

    CREATE TABLE inventory( name text, docstring text );
    CREATE TABLE definition( pkgname text,  varname text, code text, primary key ( pkgname, varname ) );
    CREATE TABLE sourcefile( pathname text primary key );
    CREATE TABLE tag( pkgname text, varname text, tag text, primary key( pkgname, varname ) );
    CREATE TABLE docs( pkgname text, varname text, docstring text, wiki text, primary key ( pkgname, varname ) );
    CREATE TABLE onload( pkgname text primary key, code text );
    CREATE TABLE entrypoint( pkgname text primary key, entrypoint text );

Storing Source Code
-------------------

The inventory could also be used to store the source code itself. This 
would be quite a convenient way to build a development environment. 
The basic idea would be to change the code columns in tables onload 
and definition to be a coderef. 

A code ref would additionally reference the source file, the source (text), 
mimetype (text) and updated dates. The minor benefit of this change is that 
there's no need for a separate list of source-files, as that could be maintained
through a view. In addition, the package imports would have to be represented 
too.::


    -- Summary information about this inventory.
    CREATE TABLE inventory( name text, docstring text );
    
    -- This inventory defines these packages.
    CREATE TABLE pkg( name text PRIMARY KEY );

    -- The package encpkgname imports the package inppkgname.
    CREATE TABLE import( encpkgname text, imppkgname text, tag text, alias text, included boolean );
    
    -- The variable pkgname/varname is defined by some code and has some documentation.
    CREATE TABLE definition( pkgname text, varname text, coderef integer, docref integer, PRIMARY KEY ( pkgname, varname ), FOREIGN KEY ( coderef ) REFERENCES code( ref ), FOREIGN KEY ( docref ) REFERENCES doc( ref ) );

    -- The documentation has a short 'docstring' and some longer wiki-text.
    CREATE TABLE doc( ref integer PRIMATY KEY, docstring text, wiki text );

    -- The variable pkgname/varname is tagged with tag.
    CREATE TABLE tag( pkgname text, varname text, tag text, PRIMARY KEY ( pkgname, varname ) );

    -- The package pkgname has some code to be executed on loading.
    CREATE TABLE onload( pkgname text primary key, coderef integer, FOREIGN KEY ( coderef ) REFERENCES code( ref ) );

    -- The code has a low-level GNX form and a high level source-code form. 
    CREATE TABLE code( ref integer PRIMARY KEY, srccoderef integer, gnxcoderef integer );

    -- Source code has a some UTF-8 text, a mime-type and the last time it was updated.
    CREATE TABLE srccode( ref integer PRIMARY KEY, source text, mimtype text, updated date );

    -- GNX code has a MinXML body and the last time it was updated.
    CREATE TABLE gnxcode( ref integer PRIMARY KEY, gnx text, updated date );



The catch here is that:-

  * Version control tools (e.g. git) would be significantly less convenient.
  * The drag and drop semantics for autoloading/autoconversion would be
    lost.

This could be alleviated by having an import/export tool that generated the 




