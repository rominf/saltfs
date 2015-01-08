#!/usr/bin/env fish
# vim: ai ts=2 sw=2 et sts=2

function suite_dpaste

  # function setup
  # end

  function test_minion
    assert_equal (__fish_salt_list_minion accepted) (command ls /salt/)
  end

end


if not set -q tank_running
  complete --do-complete='salt_common --' >/dev/null
  complete --do-complete='saltfs --' >/dev/null

  source (dirname (status -f))/helper.fish

  tank_run
end
