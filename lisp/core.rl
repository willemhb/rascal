;; bootstrapping the language.

(with
  ((fun walk ;; temporary shittier version of `walk`.
     (xform coll)
     (if (empty? coll) ()
         otherwise     (cons (xform (head coll))
                             (walk xform (tail coll)))))

   (mac let ;; temporary shittier version of `let`.
     (binds & body)
     `((fun ~(map  first binds) ~@body) ~@(map second binds))))

  ;; binding.
  (mac val
    @doc "Establishes an immutable binding."
     (name bind)
    `(var ~name (annot :const true) ~bind))

  (mac fun
    @doc "Anonymous function or coroutine."
    ((List args) & body)
    (if (has-toplevel-call? body 'yield)
      ;; expand to coroutine.
      `(lmb ~args
         (lmb (& #a)
         (let ((#r (lmb (#_) ~@body)))
           (handle (((:yield #x) (put #r resume) #x))
             (#r & #a)))))
       `(lmb ~args ~@body)))

  (mac fun
    @doc "Named function or coroutine - define or add new method."
    ((Symbol name) args & body)
    (unless (bound? name &envt)
            (add-functional-binding! name &envt))
    `(add-method!
       '~name
       ~&envt
       ~(fun args & body)))

  ;; exceptions.

  ;; coroutines.
  (mac yield
    @doc "Suspend execution of a coroutine, passing `x` to the caller."
    (x)
    `(perform :yield ~x))
