;; example of the 'main' file of a Rascal project.

(fun main
  "Example of `main` function."
  [args flags opts]
  (do (each  args   #(pr $"arg:  ${%}\n"))
      (each  flags  #(pr $"flag: ${%}\n"))
      (each* opts   #(pr $"opt:  ${%1} val: ${%2}\n"))
      (exit 0)))
