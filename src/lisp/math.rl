;; core numeric functions (basic mathematic functions defined in base.rl)
(fun sqr (x)
  (* x x))

(fun cub (x)
  (* x x x))

(fun pow ;; fast power algorithm
  (x n)
  (labl (n n b x acc 1)
    (cond (zero? n)  acc
          (even? n)  (loop (/ n 2) (sqr b) acc)
          :otherwise (loop (dec n) b (* b acc)))))

(fun fac (n)
  (labl (n n acc 1)
    (if (zero? n)
        acc
        (loop (dec n) (* acc n)))))

(fun gcd (n k) ;; euclid's algorithm for computing gcds
  (cond (< n k)
          (gcd k n)
        (zero? (mod n k)
          k
        :otherwise
          (gcd k (div n k)))))

(fun sum (xs)
  (apply + xs))

(fun prod (xs)
  (apply * xs))

;; statistics and probability
(fun mean (xs)
  (if (empty? xs)
      0
      (/ (sum xs)
         (len xs))))

;; trigonometry
(fun sin (x)
  (c-call sin :float-64 (:float-64) (x)))

(fun cos (x)
  (c-call cos :float-64 (:float-64) (x)))

(fun tan (x)
  (c-call tan :float-64 (:float-64) (x)))