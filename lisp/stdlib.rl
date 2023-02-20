;; begin stdlib.rl

;; syntax table
(def *syntax* #{})

(def for-each
  (fn (f xs)
    (if (nul? xs)
        nul
        (let (h (hd xs)
              t (tl xs))
          (f h)
          (for-each f t)))))

;; end stdlib.rl