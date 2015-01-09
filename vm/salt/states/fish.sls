shells:
  pkgrepo.managed:
    - url: http://download.opensuse.org/repositories/shells/openSUSE_13.2/
    - autorefresh: True
    - gpgcheck: False

fish:
  pkg.installed

terminfo:
  pkg.installed

man:
  pkg.installed

root:
  user.present:
    - shell: /usr/bin/fish
    - remove_groups: False

{% set fish_tank_dir = '/root/fish_tank' %}

git://github.com/terlar/fish-tank.git:
  git.latest:
    - target: {{ fish_tank_dir }}

make:
  pkg.installed

'make install':
  cmd.run:
    - creates: /usr/local/share/fish-tank/tank.fish
    - cwd: {{ fish_tank_dir }}

/root/.config/fish/config.fish:
  file.managed:
    - source: salt://config.fish
    - makedirs: True
