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
