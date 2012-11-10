#!/bin/sh
cd standard_library
for package in ginger.*
do
	for file in $package/public.auto/*.gnx
	do
		echo package = $package, file = $file
		../../apps/simplifygnx/cpp/simplifygnx -p $package -suA < ${file} > "${file}.tmp"
		mv "${file}.tmp" ${file}
	done
done