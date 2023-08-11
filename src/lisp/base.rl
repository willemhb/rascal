(use "core.rl")


;; numeric types
(type ratio   [numer denom])
(type complex [real imag])


(fun + "Add two complex numbers."
  ((complex x) (complex y))
  (let (real-x (complex::real x)
        imag-x (complex::imag x)
        real-y (complex::real y)
        imag-y (complex::imag y))
    (complex (+ real-x real-y)
             (+ imag-x imag-y))))

