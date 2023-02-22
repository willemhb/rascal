;; begin stdlib.rl

#| block comment |#

;; syntax table
(defv *syntax* (dict))

(defv add-syntax
  (fn (name xform)
    (dict-set! *syntax* name xform)))

(add-syntax 'defm
            (fn (name args &body body)
               `(add-syntax ~name
                            (fn ~args ~@body))))

(defm defn
  (name args &body body)
  `(defv ~name (fn ~args ~@body)))

(defm defc
  (name value)
  `(do (defv ~name ~value)
       (freeze! name)))

(defm and
  (&body body)
  (if (nul? body)
      true
      (if (nul? (tl body))
          (hd body)
          (let (tmp (sym))
            `(let (~tmp ~(hd body))
               (if ~tmp ~(and ~@body) ~tmp))))))

(defm or
  (() false)
  ((x) x)
  ((x & rest)
    (let (xsym (sym))
      `(let (~xsym ~x)
         (if ~xsym
             ~xsym
             (or ~@rest))))))

;; end stdlib.rl