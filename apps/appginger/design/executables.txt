User Stories for the Ginger Executables
=======================================

ginger
------
The user can start an interactive session with command-line editing by typing (where % is the prompt)

	% ginger

It starts up and shows a helpful welcome-banner that includes how to find useful documentation and how to find out licensing details.


ginger-cgi
----------
The user can use ginger-cgi to write CGI scripts. There are two ways to use this executable.

It can be used in a shebang (#!) UNIX CGI script. This would be a file that starts like this and has been marked as executable. 

	#!/usr/local/bin/ginger-cgi 	
	... SOURCE CODE GOES HERE ...
	
Alternatively it can be used in an Apache CGI handler directive. In httpd.conf you might add these lines.

	Action common /usr/local/bin/ginger-cgi -gcommon
	AddHandler common .cmn


ginger-script
-------------
The user can use this executables to run scripts in four ways. It is expected to be used as an unattended tool - so errors are NOT trapped and there are no user-customisable defaults.

Firstly they can simply execute some files. The filename extensions are used to figure out which grammar to use.

	% ginger-script FILES

Secondly, theuser can use it to execute scripts supplied on the standard input. This may be combined with file loading. The standard input is executed after the files are loaded.

	% ginger-script --stdin FILES
	
Thirdly, the user can use it to execute shebang (hash-bang) scripts. The grammar selected is based on the extension of the script's filename or may be selected using the -g,--grammar option. Here is a typical first line.

	#!/usr/local/bin/ginger-script -grammar=GRAMMAR
	
n.b. Shebang scripts are (often) limited in their options processing and a utility /usr/local/libexec/ginger/shebang is provided to circumvent any such issues.

Fourthly, the user can use it to execute a command. In this case the grammar should be specified.

	% ginger-script -grammar=common -c '"Hello, world!"'

Lastly, the user can use it to execute a named function from a package of a project. 

	% ginger-script -j PROJECT -p PACKAGE -v VARIABLE
