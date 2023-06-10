(fun not
  (x)
  (if x         true
      otherwise false))

(fun cons*
  (x y & more)
  (if (null? more) (cons x y)
      otherwise    (cons x (cons* y more))))

