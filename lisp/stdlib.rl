 ;; begin stdlib.rl

#| block comment |#

;; syntax table
(var *syntax* (dict))

(var add-syntax
  (lmb (name xform)
    (dict-set! *syntax* name xform)))

(add-syntax 'mac
            (fn (name args &va body)
              `(add-syntax ~name
                          (fn ~args ~@body))))

(mac catch
  (handler &va body)
  `(with ((raise (fn ~@handler)))
     (do ~@body)))

(fn safe-div
  (x y)
  (if (zero? y)
      (raise :division-by-zero)
      (/ x y)))

(fn uses-safe-div
  (x y z)
  (catch ((_) (resume nan))
    (+ x (safe-div y z))))

(fn map
  ([f xs]
   (if (empty? xs)
       xs
       (conj (map f xs.rst)
             (f xs.fst))))
  ([f & xs]
   (cond (empty? xs)       (raise :arity-underflow)
         (some? empty? xs) (empty xs.hd)
         :otherwise        (let [fsts   (map fst xs)
                                 rsts   (map rst xs)]
                             (conj (map f .. rsts)
                                   (f .. fsts))))))



;; end stdlib.rl