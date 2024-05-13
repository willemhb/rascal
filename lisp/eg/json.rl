;; example of a json parsing library.

(module json
  "A module for reading and writing json files."
  (import (base pc re trait/serial))
  (export (load dump)))
