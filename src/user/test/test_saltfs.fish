#!/usr/bin/env fish
# vim: ai ts=2 sw=2 et sts=2

function cat
  command cat $argv ^/dev/null
end

function ls
  command ls $argv ^/dev/null
end

function touch
  command touch $argv ^/dev/null
end

function rm
  command rm $argv ^/dev/null
end

function suite_saltfs

  function setup
    salt_manage full >/dev/null
    set minion vbox-opensuse
    set -g root /salt
    set -g minion_dir $root/$minion
    set -g __fish_salt_extracted_minion $minion
  end

  function test_minion
    assert_equal (echo (__fish_salt_list_minion accepted)) (ls $root | grep -v '^result$')
  end

  function test_minion_modules
    assert_equal (echo (__fish_salt_list_module)) (ls $minion_dir)
  end

  function test_minion_functions
    refute (ls $minion_dir/acl)
    assert (ls $minion_dir)
    assert_equal (echo (__fish_salt_list_function_without_module acl)) (ls $minion_dir/acl)
  end

  function test_minion_grains
    refute (ls $minion_dir/grains)
    assert (ls $minion_dir)
    assert_equal (echo (__fish_salt_list_grain | sort)) (ls $minion_dir/grains)
  end

  function test_minion_grain_read
    refute (cat $minion_dir/grains/id)
    assert (ls $minion_dir)
    assert (ls $minion_dir/grains)
    assert_equal (echo (__fish_salt_grain_read id)) (cat $minion_dir/grains/id)
  end

  function test_minion_grain_write
    refute (cat $minion_dir/grains/test)
    assert (ls $minion_dir)
    assert (ls $minion_dir/grains)
    assert (printf 0 >$minion_dir/grains/test)
    assert_equal 0 (cat $minion_dir/grains/test)
    assert (printf 1 >$minion_dir/grains/test)
    assert_equal 1 (cat $minion_dir/grains/test)
  end

  function test_minion_function_without_args
    refute (touch $minion_dir/test)
    assert (ls $minion_dir)
    assert (ls $minion_dir/test)
    assert (touch $minion_dir/test/ping)
  end

  function test_minion_function_with_args
    refute (ls $minion_dir/test)
    assert (ls $minion_dir)
    assert (ls $minion_dir/test)
    assert (printf 1 >$minion_dir/test/arg)
  end

end


if not set -q tank_running
  complete --do-complete='salt_common --' >/dev/null
  complete --do-complete='saltfs --' >/dev/null

  source (dirname (status -f))/helper.fish

  tank_run
end
