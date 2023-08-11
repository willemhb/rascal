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
  (let (real-x (complex-real x)
        imag-x (complex-imag x)
        real-y (complex-real y)
        imag-y (complex-imag y)
        real-z (+ real-x real-y)
        imag-z (+ imag-x imag-y))
    (complex real-z imag-z)))

(fun - "Subtract two complex numbers."
  ((complex x) (complex y))
  (let (real-x (complex-real x)
        imag-x (complex-imag x)
        real-y (complex-real y)
        imag-y (complex-imag y)
        real-z (- real-x real-y)
        imag-z (- imag-x imag-y))
    (complex real-z imag-z)))

;; example of c-call usage
(fun getenv "Get value of system environment variable."
  (variable)
  (c-call :getenv :string (:string) variable))

(fun sys "Execute string as system command."
  (command)
  (c-call :system :int (:string) command))

;; miscellaneous syntax examples
(map #`1 + %` (list 1 2 3 4)) ;; => (2 3 4 5)
(map #`% ^ 2` (list 1 2 3 4)) ;; => (1 4 9 16)
