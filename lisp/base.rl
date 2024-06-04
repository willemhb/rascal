(module base
  "The rascal base library (builtin functions)."
  (import (core io))
  (export (literal? eval repl))
  (begin
    ;; utility.
    (fun literal?
      "Return true if `x` is self-evaluating."
      (x)
      (if (symbol? x)
        (keyword? x)
        (not|list? x)))

    (fun any?
      "If function argument is missing, treat the predicate as identity."
      (xs: List) (any? id xs))

    (fun any?
      "If first argument is not a function, treat as an equality predicate."
      (x xs:List) (any? #(= %1 x) xs))

    (fun any?
      "Return true if `p` is true for at least one element of `xs`."
      (p: Function xs: List)
      (for (:from x xs
            :some (p x))))

    (fun all?
      "If function argument is missing, treat the predicate as identity."
      (xs: List) (all? id xs))

    (fun all?
      "If first argument is not a function, treat as equality test."
      (x xs:List) (all? #(= %1 x) xs))

    (fun all?
      "Return true if `p` is true for every element of `xs`."
      (p: Function xs: List)
      (for (:from x xs
            :each (p x))))

    (fun map
      "Using `for` macro."
      (f: Function xs: List)
      (for (:from x xs
            :into a ()
            :done (rev a))
        (f x)))

    (fun map
      "For Vectors."
      (f: Function xs: Vector)
      (for (:from x xs
            :into a [])
        (f x)))

    (fun filter
      "Using `for` macro."
      (p: Function xs: List)
      (for (:from x xs
            :into a ()
            :when (p x)
            :done (rev a))
        x))

    (fun drop
      "Remove the first `i` items from xs."
      (n: Small xs: List)
      (for (:from i (range n 0 -1)
            :from _ xs
            :done xs)))

    (fun drop
      "Remove items from `xs` while `p` returns `true`."
      (p: Function xs: List)
      (for (:from x xs
            :quit (not|p x)
            :done xs)))

    ;; eval and friends.
    (fun eval
      "With one argument, passes current environment to two argument form."
      (x) (eval x &env))

    (fun eval
      "With two arguments, evaluates `x` in the given environment `e`."
      (x e)
      (cond
        (literal? x)  x
        (variable? x) (lookup x e)
        otherwise     (let c (comp x e)
                        (exec c))))

    (fun repl
      "Read eval print loop (wrapped in a toplevel abort handler that resumes the repl)."
      ()
      (handle
        ((op _)
         (prl $"abort> (abort {op} ...) reached toplevel.")
         (repl))
        (let (_ (prn "rascal> ")
              x (read)
              v (eval x))
          (prl $"\nresult> {v}")
          (repl))))))
