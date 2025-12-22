;; right now this file is just for brainstorming desired syntax and whatnot. The code here
;; doesn't run and isn't intended to.

;; Note: `fun` is now a builtin that supports both forms:
;;   (fun (args) body)        - anonymous function
;;   (fun name (args) body)   - named function, defines globally

(stx let
  (binds & body)
  (def parms   (map fst binds))
  (def inits   (list-of nul (len binds)))
  (def assigns (map* #`(put ~&1 ~&2) binds))
  `((fn ~parms ~@assigns ~@body) ~@inits))

(stx* and
  (()      true)
  ((x)     x)
  ((x & r) `(let ((x# ~x))
              (if x# (and ~@r) x#))))

(stx* or
  (()      false)
  ((x)     x)
  ((x & r) `(let ((x# ~x))
              (if x# x# (or ~@r)))))

(stx* cond
  ((clause)
    (let ((test head|clause)
          (csqt tail|clause))
      (if (=? test 'otherwise)
        `(do ~@csqt)
        `(if ~test (do ~@csqt) (error "unmatched condition")))))
  ((clause & more)
    (let ((test head|clause)
          (csqt tail|clause))
      (if (=? test 'otherwise)
        (error "'otherwise appears in non-final position")
        `(if ~test
           (do ~@csqt)
           (cond ~@more))))))

;; example of full multimethod implementation
(fun pow: Num
  (x: Num n: Num)
  (label ((x x) (n n) (a 1))
    (cond
      ((zero? n) a)
      ((even? n) (loop (sqr x) (/ n 2) a))
      (otherwise (loop (* x x) (dec n) (* a x))))))

(fun fib: Num
  (n: Num)
  (label ((n n) (a 0) (b 1))
    (cond
      ((zero? n) a)
      (otherwise (loop (dec n) b (+ a b))))))
