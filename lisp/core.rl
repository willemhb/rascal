;; bootstrapping the language.

;; exceptions.
(fun cons*
  "Like cons, but if the second argument isn't a list it treats it like a list of one input."
  (x (List xs))
  (cons x xs))

(fun cons*
  "Like cons, but if the second argument isn't a list it treats it like a list of one input."
  (x y)
  (list x y))

(fun hd head)
(fun tl tail)

(fun hhd
  ((List xs))
  (head (head xs)))

(fun htl
  ((List xs))
  (head (tail xs)))

(fun thd
  ((List xs))
  (tail (head xs)))

(mac raise
  "Exception handling just wraps effect handlers."
  ((String message))
  `(perform (:exception) ~message))

(mac raise
  "Exceptions can include an exception type."
  ((Symbol xtype) (String message))
  `(perform (:exception ~xtype) ~message))

(mac raise
  "The exception type can be namespaced."
  ((List xtype) (String message))
  `(perform (:exception ~@xtype) ~message))

(mac guard
  "Evaluates body in a context where exceptions are handled by handlers in the guard clauses."
  ((List guards) & body)
  (fun expand-guard
    "Properly namespaces handler clauses."
    (guard-clause)
    (cons (cons (cons* :exception (hhd guard-clause)) (thd guard-clause)) (tl guard-clause))
  `(handle ~(map expand-guard guards) ~@body))

;; basic io library.
(fun open
  ((String path))
  (open path "rt"))

(fun open
  ((String path) (String mode))
  (guard
    ((:file-not-found msg)
     (do (prn msg) nul))
    (open-file path mode)))

