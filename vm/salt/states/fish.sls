'zypper addrepo http://download.opensuse.org/repositories/shells/openSUSE_13.1/shells.repo; zypper --gpg-auto-import-keys ref -r shells':
  cmd.run

fish:
  pkg.installed

terminfo:
  pkg.installed

root:
  user.present:
    - shell: /usr/bin/fish
    - remove_groups: False

#/etc/fish/config.fish:
## /root/.config/fish/config.fish:
#  file.append:
#    - makedirs: True
#    - text: |
#       set -x TERM vt100
