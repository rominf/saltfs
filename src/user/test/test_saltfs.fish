#!/usr/bin/env fish
# vim: ai ts=2 sw=2 et sts=2

# function ls
#   command ls $argv
# end

function suite_dpaste

  function setup
    salt_manage full
    set -g __fish_salt_extracted_minion minion
  end

  function test_minion
    assert_equal (echo (__fish_salt_list_minion accepted)) (ls /salt)
  end

  function test_minion_modules
    assert_equal (echo (__fish_salt_list_module)) (ls /salt/minion)
  end

  function test_minion_functions
    refute ls /salt/minion/acl
    assert ls /salt/minion
    # assert_equal (echo (__fish_salt_list_function_without_module acl)) (ls /salt/minion/acl)
  end

  function test_minion_grains
    refute ls /salt/minion/grains
    assert ls /salt/minion
    # assert_equal (echo (__fish_salt_list_grain)) (ls /salt/minion/grains)
  end

end


if not set -q tank_running
  complete --do-complete='salt_common --' >/dev/null
  complete --do-complete='saltfs --' >/dev/null

  source (dirname (status -f))/helper.fish

  tank_run
end
