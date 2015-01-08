function suite_list_equality
    function setup
        set -g test_dir (stub_dir)
        touch $test_dir/(seq 3)
    end

    function test_list_assertions
        set -l IFS
        assert_equal (echo -e "1\n2\n3") (ls $test_dir)
    end
end

if not set -q tank_running
    set -l fish_tank /usr/local/share/fish-tank/tank.fish
    if not test -e $fish_tank
        echo 'error: fish-tank is required to run these tests (https://github.com/terlar/fish-tank)'
        exit 1
    end

    source $fish_tank
    tank_run
end
