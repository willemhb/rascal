(ns re
  "Rascal regular expression library."

  (val ;; Regex flags.
    ignore-case #x01
    debug       #x02
    dotall      #x04
    verbose     #x08
    locale      #x10)

  (struct Regex
    "Stores a compiled regular expression."
    [flags: Small,
     pattern: String,
     exec: Binary]))