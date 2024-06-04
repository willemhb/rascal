(module base
  "The rascal base library (builtin functions)."
  (import (core io))
  (export (literal? eval repl))
  (begin
    ;; utility.
    (fun literal?
      "Return true if `x` is self-evaluating."
      (x)
      (if
        (symbol? x)
        (keyword? x)
        (not (list? x))))

    ;; eval and friends.
    (fun eval
      "With one argument, passes current environment to two argument form."
      (x)
      (eval x &env))

    (fun eval
      "With two arguments, evaluates `x` in the given environment `e`."
      (x e)
      (cond
        ((literal? x)  x)
        ((variable? x) (lookup x e))
        (otherwise     (let ((compiled (compile x e)))
                         (exec compiled)))))

    (fun repl
      "Read eval print loop (wrapped in a toplevel abort handler that resumes the repl)."
      ()
      (handle
        ((op _)
         (prn $"abort> (abort {op} ...) reached toplevel.\n")
         (repl))
        (let ((_ (prn "rascal> "))
              (x (read))
              (v (eval x)))
          (prn $"\nresult> {v}\n")
          (repl))))))
