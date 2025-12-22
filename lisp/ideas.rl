;; right now this file is just for brainstorming desired syntax and whatnot. The code here
;; doesn't run and isn't intended to.

;; Note: `fun` is now a builtin that supports both forms:
;;   (fun (args) body)        - anonymous function
;;   (fun name (args) body)   - named function, defines globally

(stx let
  (binds & body)
  `((fun ~(map fst binds) ) ~@(map snd binds)))

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
    (let ((test clause|head)
          (csqt clause|tail))
      (if (=? test 'otherwise)
        `(do ~@csqt)
        `(if ~test (do ~@csqt) (error "unmatched condition")))))
  ((clause & more)
    (let ((test clause|head)
          (csqt clause|tail))
      ())))