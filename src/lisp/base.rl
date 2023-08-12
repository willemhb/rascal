(use "core.rl")


;; example of type definitions and method implementations
;; struct types
(struct ratio (numer denom))
(struct complex (real imag))

;; union types
(union number (real small big ratio complex))

;; record types
(record person (name age gender))

;; implementations of basic arithmetic for the complex type
(fun + "Add two complex numbers."
  ((complex x) (complex y))
  (let ((real-x (complex-real x))
        (imag-x (complex-imag x))
        (real-y (complex-real y))
        (imag-y (complex-imag y))
        (real-z (+ real-x real-y))
        (imag-z (+ imag-x imag-y)))
    (complex real-z imag-z)))

(fun - "Subtract two complex numbers."
  ((complex x) (complex y))
  (let ((real-x (complex-real x))
        (imag-x (complex-imag x))
        (real-y (complex-real y))
        (imag-y (complex-imag y))
        (real-z (- real-x real-y))
        (imag-z (- imag-x imag-y)))
    (complex real-z imag-z)))

;; example of c-call usage
(fun getenv "Get value of system environment variable."
  (variable)
  (c-call :getenv :string (:string) variable))

(fun sys "Execute string as system command."
  (command)
  (c-call :system :int (:string) command))

;; example of effect usage
(fun safe-div
  (x y)
  (if (zero? y)
      (perform (:exception :zero-division) (x y))
      (/ x y)))

(fun use-safe-div
  (x y)
  (handle
    ((((:exception :zero-division) & _) (do (print "Zero division in use-safe-div.") nul)))
    (safe-div x y))

;; using macros and effects to create new control structures
(mac catch "Simple try/catch implementation."
  (handlers & body)
  (let ((transform (lmb (handler)
                    (let ((op   (hhd handler))
                          (args (thd handler))
                          (body (tl  handler)))
                      (cond ((list? op) `(((:exception ~@op) ~@args) ~@body))
                            (otherwise  `(((:exception ~op) ~@args) ~@body))))))
        (handlers  (map transform handlers)))
    `(handle ~handlers ~@body)))

(mac raise "Companion to catch."
  (exception-type & args)
  (if (list? exception-type)
      `(perform (:exception ~@exception-type) ~args)
      `(perform (:exception ~exception-type) ~args)))

(fun safe-division "Similar to safe-div but uses catch/raise syntax."
  (x y)
  (if (zero? y)
      (raise :zero-division x y)
      (/ x y)))

(fun use-safe-division "Etc."
  (x y)
  (catch
    (((:zero-division x y)
      (do (print "Zero-division in (use-safe-division ~a ~a)" x y)
          nul)))
    (safe-division x y)))
