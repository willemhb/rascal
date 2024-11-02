(ns re
  "Rascal regular expression library."

  (val ;; Regex flags.
    ignore-case #x01
    debug       #x02
    dotall      #x04
    verbose     #x08
    locale      #x10)

  (struct Re
    "Stores a compiled regular expression."
    [flags:   Small,
     pattern: Str,
     exec:    Bin])

  (struct Match
    "Stores the result of a regular expression match."
    [regex:  Re,
     string: Str,
     start:  Small,
     stop:   Small,
     groups: List])

  (fun match: Opt[Match]
    "Applies the search function designed by a regex to given string."
    [r: Re, s: Str]
    ()))