# ssh-rsa AAAAB3NzaC1yc2EAAAADAQABAAACAQC2pOSMnx02u1EiNa4LL7OQhOSqxUbV7vQgXmotvhyR5/0bPHc8V4miGrrPKBkpyrnEUXkHpdLaBDe4sXb8Z4tNTSCTfCRB4QNc4AovqI/kKnbIr7cDdtnyKrk7Zw7DzIKi4c+Bn/jdrzon7MkQpVPItq1U+gppjSj8QUCg5D+pMZwkwqaLjQ7FmfAFLDVWTtgg1ZN5NoYnWyiRKrfXe90vLpxjRuT2bpsz17sRMxL+ZjuL8vumi5tXNL27UQLEQUO2fGkB4Q9m13LxFoKjk3w9y2t+GvWxugaY5g+YRanARlKF90iiMvaP5PrgZJeD/szHFgtUbEBgjMeTvc3IEQEZD8yxLR2aqwqO7+uHgOVdcS5qpu/+/oqMeB+lWiX21dVgItND3EhPL4N1PZPbjEd7H+5UE3ZRPPfHwGdiXnQJ1xAw5ga69qEc0YRcsV2sKxftHf9wy6bzP5O5j+mZJaN1T5e6oqtb7W16lqT1892ecyDfqwurLxwBZnrYFD6kdYyQuPb5Wgi9wZu84PK1X/YaYeTHQk9L4jyrajidqO+eQ80+wiEakwXMlkOC+itBFUjOpGf2nn9k6rYtFtWuGD8zsJgsJDu3mjJczWXH98JEiWQJS5D+8EaLUNruJuPSTZzVOomVPUwzOhrmWLfZ4ZBRLQLju6tp4ONxOU1rU/6ZsQ== romas@romas-x230-suse.lan:
#   ssh_auth:
#     - present
#     - user: root

sshkey:
  ssh_auth.present:
    - user: root
    - source: salt://sshkey/host_id_rsa.pub
