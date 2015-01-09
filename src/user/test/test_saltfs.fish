#!/usr/bin/env fish
# vim: ai ts=2 sw=2 et sts=2

function ls
  command ls $argv ^/dev/null
end

function suite_saltfs

  function setup
    salt_manage full >/dev/null
    set -g __fish_salt_extracted_minion minion
  end

  function test_minion
    assert_equal (echo (__fish_salt_list_minion accepted)) (ls /salt)
  end

  function test_minion_modules
    assert_equal (echo (__fish_salt_list_module)) (ls /salt/minion)
  end

  function test_minion_functions
    refute (ls /salt/minion/acl)
    assert (ls /salt/minion)
    assert_equal (echo (__fish_salt_list_function_without_module acl)) (ls /salt/minion/acl)
  end

  function test_minion_grains
    refute (ls /salt/minion/grains)
    assert (ls /salt/minion)
    assert_equal (echo (__fish_salt_list_grain | sort)) (ls /salt/minion/grains)
  end

  function test_minion_grain_read
    refute (cat /salt/minion/grains/id)
    assert (ls /salt/minion)
    assert (ls /salt/minion/grains)
    assert_equal (echo (__fish_salt_grain_read id)) (cat /salt/minion/grains/id)
  end

  function test_minion_grain_write
    refute (cat /salt/minion/grains/test)
    assert (ls /salt/minion)
    assert (ls /salt/minion/grains)
    assert (printf 0 >/salt/minion/grains/test)
    assert_equal 0 (cat /salt/minion/grains/test)
    assert (printf 1 >/salt/minion/grains/test)
    assert_equal 1 (cat /salt/minion/grains/test)
  end

  function test_minion_grain_rm
    refute (rm /salt/minion/grains/test)
    assert (ls /salt/minion)
    assert (ls /salt/minion/grains)
    assert (rm /salt/minion/grains/test)
    refute (ls /salt/minion/grains/test)
  end

  function test_minion_function_without_args
    refute (touch /salt/minion/test)
    assert (ls /salt/minion)
    assert (ls /salt/minion/test)
    assert (touch /salt/minion/test/ping)
  end

  function test_minion_function_with_args
    refute (ls /salt/minion/test)
    assert (ls /salt/minion)
    assert (ls /salt/minion/test)
    assert (printf 1 >/salt/minion/test/arg)
  end

end


if not set -q tank_running
  complete --do-complete='salt_common --' >/dev/null
  complete --do-complete='saltfs --' >/dev/null

  source (dirname (status -f))/helper.fish

  tank_run
end
