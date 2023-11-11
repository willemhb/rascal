#| functions and macros for bootstrapping the language.
   mostly very simple core utilities, core syntax, and helpers for syntactic analysis.

 |#

;; exceptions.
(mac raise
  "Invoke the nearest enclosing exception handler bound by a `guard` form."
  (& args)
  `(perform :exception/raise ~args))

(mac guard
  "Bind a new exception handler."
  ((List handler) & body)
  (let (([[o a] :as a* & h] handler))
    `(handle
       (~a*
        (if (id? ~o :exception/raise)
            (do ~@h)
            (perform ~o ~a)))
       ~@body)))

;; generators and coroutines.
(mac yield
  "Suspend execution of a generator and pass `arg` to caller."
  (arg)
  `(perform :generator/yield (~arg)))

(mac yield
  "Implicit `nul`."
  ()
  `(perform :generator/yield (nul)))

(mac generator
  "Create a new suspendable function."
  ((List spec) & body)
  `(fun ~spec
     (var #r0 nul)
     ((fun (#x)
        (handle
          ((#o #a #r1)
           (if (id? #o :generator/yield)
               (do (put #r0 #r1) (hd #a))
               (perform #ro #a #r1)))
          (if (nul? #r0)
              (do ~@body)
              (#r0 #x)))) nul)))