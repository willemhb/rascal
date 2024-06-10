(module base
  "The rascal base library (builtin functions)."
  (import [core io])
  (export [Function, Callable,
           id, const, curry, literal?,
           any?, all?, map, filter, take, drop, fold,
           eval, repl])

  (begin
    ;; types.
    (type Function
      "Simple union of function types."
      (Closure Native Generic))

    (type Callable
      "Union of Function and Type."
      (Function Type))

    ;; utility.
    (fun id
      "Return `x` unchanged."
      [x] x)

    (fun const
      "Return a function that always returns `x`."
      [x] (fun [_] x))

    (fun curry
      "Returna function that calls `f` with `xs` as its initial argument(s)."
      [f: Function, & xs]
      (fun [& ys] (apply f xs ys)))

    (fun literal?
      "Return true if `x` is self-evaluating."
      [x]
      (if (symbol? x)
        (keyword? x)
        (!list? x)))

    (fun any?
      "If function argument is missing, treat the predicate as identity."
      [xs: List] (any? id xs))

    (fun any?
      "If first argument is not a function, treat as an equality predicate."
      [x, xs: List] (any? #(= % x) xs))

    (fun any?
      "Return true if `p` is true for at least one element of `xs`."
      [p: Function, xs: List]
      (cond
        (done? xs) false
        (p|hd xs)  true
        otherwise  (any? p (tl xs))))

    (fun all?
      "If function argument is missing, treat the predicate as identity."
      [xs: List] (all? id xs))

    (fun all?
      "If first argument is not a function, treat as equality test."
      [x: Any, xs: List] (all? #(= % x) xs))

    (fun all?
      "Return true if `p` is true for every element of `xs`."
      [p: Function, xs: List]
      (cond
        (done? xs) true
        (!p|hd xs) false
        otherwise  (all? p (tl xs))))

    ;; map implementations
    (fun map
      "Treat single argument forms as curried calls."
      ([f: Function] (fun [xs] (map f xs)))
      ([n: Small]    (fun [xs] (map #(ref % n) xs)))
      ([s: Symbol]   (fun [xs] (map #(ref % s) xs)))
      ([s: String]   (fun [xs] (map #(ref % s) xs))))

    (fun map
      "Defined for Lists."
      [f: Function, xs: List]
      (label [xs xs, ac ()]
        (if (done? xs)
          (rev ac)
          (let [[x & xs] xs]
            (loop xs (conj ac (f x)))))))

    (fun map
      "Defined for Vectors."
      [f: Function, xs: Vector]
      (label [s (seq xs), ac #[]]
        (if (done? s)
          (freeze ac)
          (loop (rest s) (conj ac (f|first s))))))

    (fun map
      "Defined for Maps."
      [f: Function, xs: Map]
      (label [s (seq xs), ac #{}]
        (if (done? s)
          (freeze ac)
          (let [(k . v) (first s)]
            (loop (rest s) (conj ac k (f v)))))))

    (fun filter
      "Using `for` macro."
      [p: Function, xs: List]
      (label [xs xs, ac ()]
        (if (done? xs)
          (rev ac)
          (let [[x & xs] xs]
            (if (p x)
              (loop xs (cons x ac))
              (loop xs ac))))))

    (fun drop
      "Remove the first `i` items from `xs`."
      [n: Small, xs: List]
      (cond
        (done? xs) ()
        (zero? n)  xs
        otherwise  (drop (1- n) (tl xs))))

    (fun drop
      "Remove items from `xs` while `p` returns `true`."
      [p: Function, xs: List]
      (cond
        (done? xs) ()
        (p (hd x)) (drop p (tl xs))
        otherwise  xs))

    (fun take
      "Return a new list containing the first `i` items from `xs`."
      [n: Small, xs: List]
      (label [xs xs, n n, ac ()]
        (cond
          (done? xs) (rev ac)
          (zero? n)  (rev ac)
          otherwise  (loop (tl xs) (1- n) (cons (hd xs) ac)))))

    ;; eval and friends.
    (fun eval
      "With one argument, passes current environment to two argument form."
      [x] (eval x &env))

    (fun eval
      "With two arguments, evaluates `x` in the given environment `e`."
      [x, e]
      (cond
        (literal? x)  x
        (variable? x) (lookup x e)
        otherwise     (let [c (comp x e)]
                        (exec c))))

    (fun repl
      "Read eval print loop (wrapped in a toplevel abort handler that resumes the repl)."
      []
      (handle
        ([op, _]
         (prl $"abort> (abort {op} ...) reached toplevel.")
         (repl))
        (let [_ (prn "rascal> "),
              x (read),
              v (eval x)]
          (prl $"\nresult> {v}")
          (repl))))))
