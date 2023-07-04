(def not
  (lmb (x)
    (if x         true
        otherwise false)))

(def cons*
  (lmb (x y & more)
    (if (nil? more) (cons x y)
        otherwise   (cons x
                          (cons* y .. more)))))

