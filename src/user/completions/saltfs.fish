# Argh!1!1!! This is duct tape. Check this function in the
# /usr/share/fish/salt_common.fish file, and tell me why does it
# work from the user space, but not from the kernel space and
# WHY does it work just fine with temp files?!
# OK, you can just fix it without telling me why.
function __fish_salt_exec_and_clean
	set tmpfile (mktemp)
	__fish_salt_exec_output $argv >$tmpfile
	and cat $tmpfile | __fish_salt_clean $argv[1]
	rm $tmpfile
end

function __fish_salt_list_function_without_module
	set tmpfile (mktemp)
	__fish_salt_list_function $argv >$tmpfile
	and cat $tmpfile | sed "s/^.*\.//g"
	rm $tmpfile
end

function __fish_salt_grain_read
	__fish_salt_exec_and_clean nested grains.get $argv
end
