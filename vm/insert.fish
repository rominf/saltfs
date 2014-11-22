#!/usr/bin/env fish

if contains force $argv
	set rmmod_args -- '--force'
end

cp -f ../src/kern/saltfs.ko ../bin/
vagrant ssh -c "rmmod --force saltfs; insmod /module/saltfs.ko"
#vagrant ssh -c "rmmod $rmmod_arg saltfs; insmod /module/saltfs.ko"
