(ns base
  "Main rascal standard library file."
  (use [core])

  ;; coroutine example.
  (fun range
    [e]
      (range 0 e 1)
    [b e]
      (range b e 1)
    [b e s]
      (let r nul
        (hndl
          [k o x]
          (if (= o :yield)
            (do (put r k) x)
            (raise k o x))
          (label
            [b b, e e, s s]
            (if (>= b e)
              e
              (do (yield b)
                (loop (+ b s) e s)))))))

  ;; eval and friends.
  (fun eval
    "Evaluate `x` in the given `Env` (use current `Env` if not supplied)."
    [x]
      (eval x &env)
    [x e]
      (cond
        (lit? x)  x
        (sym? x)  (lookup x e)
        otherwise (let c (compile x e)
                    (exec c))))

  (fun repl
    "read/eval/print loop."
    []
    (catch
      ;; Toplevel error handler.
      [e]
      (do (pr $"error> ${e}.\n")
        (repl))
      (let _ (pr "rascal> ")
           x (read)
           v (eval x)
           _ (pr $"\nresult> ${v}\n")
        (repl)))))