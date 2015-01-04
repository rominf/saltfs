#!/usr/bin/env fish

echo cleaning all
rm -f .saltfs.ko.cmd
rm -rf .tmp_versions/
rm -f Module.symvers
rm -f saltfs.mod.c
rm -f saltfs.ko
rm -f .*.o*
rm -f *.o*
