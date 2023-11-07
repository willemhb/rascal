;; bootstrapping the base language.
(load "prelude.rl")

;; binding forms
@final
@type Function
@macro
@doc "Standard syntax for creating new macro bindings."
@name 'mac
(def mac
  (lmb ((List &form) (Environment &envt) (Symbol name) (List args) & body)
    (annot (list 'def
                 name
                 (cons 'lmb
                       (cons (list 'List '&form) (list 'Environment '&envt) args)
                       body)))
           :macro true
           :type  Function
           :final true
           :name  name))

@doc "Support docstring."
(mac mac
  ((Symbol name) (String doc) (List args) & body)
  (annot (list 'def
               name
               (cons 'lmb
                     (cons (list 'List '&form) (list 'Environment '&envt) args)
                     body))
         :macro true
         :type  Function
         :final true
         :doc   doc
         :name  name))

(mac fun
  "Standard syntax for creating new function bindings and anonymous functions."
  ((List args) & body)
  (cons 'lmb args body))

(mac fun
  "If a name is provided, expand to `(def name (lmb args & body))`."
  ((Symbol name) (List args) & body)
  (annot (list 'def
              name
              (cons 'lmb args body))
         :type  Function
         :final true
         :name  name))

(mac fun
  "Support docstrings."
  ((Symbol name) (String doc) (List args) & body)
  (annot (list 'def
               name
               (cons 'lmb args body))
         :type  Function
         :final true
         :doc   doc
         :name  name))

(mac var
  "Standard syntax for introducing mutable bindings."
  ((Symbol name) value)
  (list 'def name value))

(mac var
  "Vars can have type constraints."
  ((List spec) value)
  (annot (list 'def (hd spec) value) :type (tl spec)))

(mac val
  "Standard syntax for introducing immutable bindings."
  ((Symbol name) value)
  (annot (list 'def name value) :final true))

(mac val
  "Constants can have type constraints."
  ((List spec) value)
  (annot (list 'def (hd spec) value) :final true :type (tl spec)))

(mac struct
  "Standard syntax for introducing new structure bindings."
  ((Symbol name) (List slots))
  (annot (list 'def
               name
               (list 'ensure-type
                     'Struct
                     (list 'quote slots)))
         :final true
         :type  Type
         :name  name))

(mac raise
  "Standard syntax for raising an exception."
  ((List args))
  `(perform (:exception) ~args))

(mac raise
  "Support passing a continuation explicitly."
  ((List args) resume)
  `(perform (:exception) ~@args resume))

(mac raise
  "Exception types can be namespaced."
  ((Symbol exception-type) (List args))
  `(perform (:exception ~exception-type) ~@args))

(mac raise
  "And a continuation can be passed as well."
  ((Symbol exception-type) (List args) resume)
  `(perform (:exception ~exception-type) ~args resume))

(mac guard
  "Standard syntax for handling exceptions."
  ((List guard-spec) & body)
  #| usage

     (fun safe-divide
       "Checked division. Prints a message and resumes with nan."
       (a b)
       (guard
         ((e a r)
            (case (hd|tl e)
              ((:zero-division)
               (prln "Tried to divide ~a by 0." (hd a))
               (r nan))
              ((:type-error)
               (prln "Non-number in `(/ ~a ~a)`." (hd|tl a) (hd|tl|tl a))
               (r nan))
              ((_)
               (raise (hd|tl e) args r))))
         (cond ((or (not|number? a) (not|number? b))
                (raise :type-error (a b)))
               ((zero? b)
                (raise :zero-division (a)))
               (otherwise
                (/ a b)))))

   |#
  (let (((args-spec & handler) guard-spec)
        ((op-name & op-args)   args-spec))
    `(handle
       (~args-spec
         (if (id? :exception (fst* ~op-name))
             (do ~@handler)
             (perform ~op-name ~@op-args)))
       ~@body)))

(mac yield
  "Suspends execution of a coroutine, passing the supplied value."
  (arg)
  `(perform :generator/yield (~arg)))

(mac yield
  "Supports zero-argument verison (implicit `nul`)."
  ()
  `(perform :generator/yield (nul)))

(mac generator
  "Standard syntax for creating a new generator."
  #|
    usage

    (generator range
      "Generate all the integers between `start` and `stop`, incrementing by `step`."
      (start stop step)
      (label loop ((start start))
        (if (>= start stop)
            stop
            (do (yield start)
                (loop (+ start step))))))

   |#
  ((List formals) & body)
  `(lmb ~formals
     ((lmb (#x)
        (let ((#resume0 nul))
          (handle ((#op #args #resume1)
                   (if (id? #op :generator/yield)
                       (do (put #resume0 #resume1) (hd #args))
                       (perform #op #args #resume1)))
                  (if (nul? #resume0)
                      (do ~@body)
                      (#resume0 #x)))))) nul))

(mac generator
  "Support named generators."
  ((Symbol name) (List formals) & body)
  `(def ~name (generator ~formals ~@body)))

(mac generator
  "Support docstring in named generators."
  ((Symbol name) (String doc) (List formals) & body)
  (annot `(def ~name (generator ~formals ~@body)) :doc doc))

(mac for
  "Standard collection looping syntax."
  #|
     usage

     (for i in '(1 2 3 4) do
       (prln i) i)
   |#

  ((Symbol loop-variable) in iterable do & body)
  `(label #loop ((~loop-variable nul)
                 (#iterable ~iterable)
                 (#result nul))
     (if (empty? #iterable)
         #result
         (do (put ~loop-variable (fst #iterable))
             (put #iterable      (rst #iterable))
             (put #result        (do ~@body))
             (#loop ~loop-variable #iterable #result)))))

(mac while
  "Standard conditional looping syntax."
  #|
     usage

     (let ((i 0))
       (while (< i 10) do
         (prln i)
         (put i (inc i))))
   |#

  (test do & body)
  `(label #loop ((#test   ~test)
                 (#result nul))
     (if #test
         #result
         (do (put #result (do ~@body))
             (put #test ~test)
             (#loop #test #result)))))

(mac while*
  "Post-test looping syntax."
  #|
     usage

     (let ((i 0))
       (while* (> i 0) do
         (prln i)
         (put i (dec i))))
   |#

   (test do & body)
   `(label #loop ((#test true)
                  (#result nul))
      (put #result (do ~@body))
      (put #test ~test)
      (if #test
          (#loop #test #result)
          #result)))

