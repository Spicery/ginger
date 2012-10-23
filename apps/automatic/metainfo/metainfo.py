import subprocess
import json
import os

################################################################################
#   generateMetaInfoTable
################################################################################

def metaInfoDir():
	"""Returns the directory for the metainfo"""
	# For development we use tmp_metainfo rather than metainfo.
	return "../automatic/tmp_metainfo"

def generateMetaInfo( ):
	metainfo = json.loads( subprocess.check_output( [ "../appginger/cpp/ginger-info", "-j" ] ) )
	stdinfo = metainfo[ "std" ]
	dir = metaInfoDir()
	if not os.path.exists( dir ):
		os.makedirs( dir )	
	return

generateMetaInfo()