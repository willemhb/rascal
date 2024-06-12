(ns sys
  "Rascal runtime interface."

  (val &args
    "Shortcut for command line arguments."
    (*command-line-arguments*))

  (val &flags
    "Shortcut for command line flags."
    (*command-line-flags*))

  (val &opts
    "Shortcut for command line options."
    (*command-line-options*)))