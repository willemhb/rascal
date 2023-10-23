;; most of the rascal standard library defined here.

;; collection utilities.
(fun take
  "Behavior is like `takewhile` if the first input is a `Function`."
  ((Function p?) (List xs))
  (label loop ((xs xs)
               (acc ()))
    (cond ((empty? xs)  (rev acc))
          ((p? (hd xs)) (loop (tl xs) (cons (hd xs) acc)))
          (otherwise    (rev acc)))))

(fun take
  "Behavior is traditional if first input is an `Integer`."
  ((Integer i) (List xs))
  (label loop ((xs xs)
               (i i)
               (acc ()))
    (cond ((empty? xs)  (rev acc))
          ((p? (hd xs)) (loop (tl xs) (cons (hd xs) acc)))
          (otherwise    (rev acc)))))

(fun drop
  "Behavior is like `dropwhile` if the first input is a `Function`."
  ((Function p?) (List xs))
  (cond ((empty? xs)  ())
        ((p? (hd xs)) (drop p? (tl xs)))
        (otherwise    xs)))

(fun drop
  "Behavior is traditional `drop` if the first input is a `Function`."
  ((Integer i) (List xs))
  (cond ((empty? xs) ())
        ((zero? i)   xs)
        (otherwise   (drop (dec i) (tl xs)))))

()

;; operating system interface.
(fun environ
  ((String k))
  (c-call getenv String (String) (v)))

(fun environ
  ((String k) (String v))
  (c-call putenv String (String String) (k v)))

@final
(fun pwd
  ()
  (c-call pwd String () ()))

(val &home (environ "HOME"))
(val &path (environ "PATH"))
(val &lang (environ "LANG"))
(val &user (environ "USER"))
(val &host (environ "HOSTNAME"))

