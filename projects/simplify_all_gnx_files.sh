#!/bin/sh
cd standard_library
for package in ginger.*
do
	if [ -d "${package}/public.auto" ]
	then
		for file in "${package}"/public.auto/*.gnx
		do
			# Run the simplifier on the files in public.auto for the package.
			../../apps/simplifygnx/cpp/simplifygnx -p $package -suA < "${file}" > "${file}.tmp"
			mv "${file}.tmp" ${file}
		done
	fi
done