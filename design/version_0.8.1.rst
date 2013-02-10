Ginger Version 0.8.1
====================

Shell and Help Commands added to Ginger interpreter via new Tool
----------------------------------------------------------------
The ginger intepreter now specially intercepts certain inputs and treats 
them as commands. These

Unix shell command - any input starting with an exclamation mark is now
handed off to the shell. e.g.

	>>> !pwd
	/Users/steve/projects/Spicery/ginger/design

Help command - any input matching 'help' or 'help <topic-name>' will
now open a web browser on that topic. e.g.

	>>> help instruction_set


User Configurable File-Extension-to-Parser Mapping
---------------------------------------------------

The file2gnx tool is now responsible for managing the mapping from file
extensions through to the parsing tool. This is now fully configurable by
the user via the settings file 'parser-mapping.mnx'. The search for settings
file is [XDG base directory specification][xdg] compliant.

[xdg]: http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html

file2gnx searches the below path for 'parser-mapping.mnx' files. The
first matching entry wins.

    [1] $XDG_CONFIG_HOME/ginger
    [2] if $XDG_CONFIG_HOME does not exist fall back to $HOME/.config/ginger
    [3] For folder FOLDER in the colon-separated path $XDG_CONFIG_PATH 
        search FOLDER/ginger
    [4] If $XDG_CONFIG_PATH is not defined fall back to /etc/ginger


Pass Multiple, Comma-Separated Options with -O Option
-----------------------------------------------------
A new -O option for passing multiple options as a single option has been
added to ginger-script, ginger-cgi and ginger. This was added
due to the widesperad limitation of only being able to support passing a
single option to #! scripts.

e.g.
    #!/usr/local/bin/ginger-script -O-ggnx,-m1


Ginger Instruction Set Documented
---------------------------------
An initial documentation set for the Ginger VM instruction set has been written.
This is automatically generated from the comments contained within the 
instructions.

In later versions this will be integrated fully into the Ginger Documentation
project. For this release the documentation is stored by default at:

	/usr/local/share/ginger/html/instruction_set.html