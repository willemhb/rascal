 ;; begin stdlib.rl

#| block comment |#

;; syntax table
(defv *syntax* (dict))

(defv add-syntax
  (fn (name xform)
    (dict-set! *syntax* name xform)))

(add-syntax 'defm
            (fn (name args &va body)
               `(add-syntax ~name
                            (fn ~args ~@body))))

(defm catch
  (handler &va body)
  `(with ((raise (fn ~@handler)))
     (do ~@body)))

(defn safe-div
  (x y)
  (if (zero? y)
      (raise :division-by-zero)
      (/ x y)))

(defn uses-safe-div
  (x y z)
  (catch ((_) (resume nan))
    (+ x (safe-div y z))))

;; end stdlib.rl