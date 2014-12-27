#!/usr/bin/env fish

# if contains force $argv
# 	set rmmod_args -- '--force'
# end

set root "../"

cp -f $root/src/kern/saltfs.ko $root/bin/

set cmd "fish -c 'sleep 0"
contains umount $argv; and set cmd "$cmd; and umount /salt; true"
contains rmmod  $argv; and set cmd "$cmd; and rmmod --force saltfs; true"
contains insmod $argv; and set cmd "$cmd; and insmod /module/saltfs.ko"
contains mount  $argv; and set cmd "$cmd; and mount -t saltfs saltfs /salt"
set cmd "$cmd'"
vagrant ssh -c $cmd

#vagrant ssh -c "rmmod $rmmod_arg saltfs; insmod /module/saltfs.ko"
