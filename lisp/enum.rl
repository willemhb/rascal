(use (base math))

;; Sequence and enumeration utilities.
(fun range
  @doc "Generate every integer between `0` and `stop`."
  ((Integer stop))
  (range 0 stop 1))

(fun range
  @doc "Generate every integer between `start` and `stop`."
  ((Integer start) (Integer stop))
  (range start stop 1))

(fun range
  @doc "Generate every `step`th integer between `start` and `stop`."
  ((Integer start) (Integer stop) (Integer step))
  (loop ((i start))
    (if (>= i stop)
      stop
      (do (yield i)
          (loop (+ i step))))))

(fun members
  @doc "Generate every `x` of `xs`."
  ((List xs))
  (loop ((xs xs))
    (case (xs)
      (())       nul
      ((x))      x
      ((x & xs)) (do (yield x)
                     (loop xs)))))

(fun for-each
  @doc "Call `f` on the elements of `xs` in order. Return the result of the last evaluation."
  ((Function f) (List xs))
  (case (xs)
     (())         nul
     ((x))        (f x)
     ((x & more)) (do (f x)
                      (for-each f more))))