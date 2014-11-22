shells:
  pkgrepo.managed:
    - url: http://download.opensuse.org/repositories/shells/openSUSE_13.2/
    - autorefresh: True
    - gpgcheck: False

fish:
  pkg.installed

terminfo:
  pkg.installed

root:
  user.present:
    - shell: /usr/bin/fish
    - remove_groups: False
