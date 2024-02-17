(module base
  "The rascal base library (builtin functions)."
  (import (core))
  (export (eval repl))
  (begin
    ;; eval and friends.
    (fun eval
      "With one argument, passes current environment to two argument form."
      (x)
      (eval x &env))

    (fun eval
      "With two arguments, ."
      (x e)
      (cond
        ((literal? x)  x)
        ((variable? x) (lookup x e))
        (otherwise     (let ((compiled (compile x e)))
                         (exec compiled)))))

    (fun repl
      "Read eval print loop (wrapped in a toplevel abort handler that resumes the repl."
      ()
      (handle
        ((op _)
         (prn "(abort " op " ...) reached toplevel.\n")
         (repl))
        (prn "rascal> ")
        (let ((x (read))
              (v (eval x)))
          (prn "\nresult> " v)
          (repl))))))
