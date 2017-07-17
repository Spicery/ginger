%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
Well-known Locations for Developers
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

The following environment variables are handy names for folders
that Ginger developers frequently use. Scripts should not rely on these
environment variables being defined externally; if they are needed they
should be defined as part of the script (or set by a parent script). Their 
purpose is to give standard names to these familiar locations.

.. code-block:: bash

	# Suggested location.
	export SPICERY_DEV_HOME=${HOME}/projects/Spicery
	export GINGER_DEV_HOME=${SPICERY_DEV_HOME}/ginger
	export GINGER_DEV_DOCS=${SPICERY_DEV_HOME}/gingerdocs

	# Suggested location.
	export GINGER_HOME=/usr/local
	export GINGER_SHARE=${GINGER_HOME}/share/ginger
	export GINGER_LIBEXEC=${GINGER_HOME}/libexec
	export GINGER_EXEC=${GINGER_HOME}/bin
