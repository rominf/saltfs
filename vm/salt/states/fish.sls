{% if not salt['pkg.version']('fish') %}
'zypper addrepo http://download.opensuse.org/repositories/shells/openSUSE_13.1/shells.repo; zypper --gpg-auto-import-keys ref -r shells':
  cmd.run
{% endif %}

fish:
  pkg.installed

terminfo:
  pkg.installed

root:
  user.present:
    - shell: /usr/bin/fish
    - remove_groups: False
