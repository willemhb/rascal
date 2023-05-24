(fun safe-divide
  (x y)
  (if (==? y 0)
      (raise :division-by-zero)
      (/ x y)))

(catch ((cause)
        (case cause
          :division-by-0 nil
          otherwise      0)))
          