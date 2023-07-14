;; macro implementation
(def :base *syntax* {})

(def set-syntax
  (lmb (name transformer)
    (dict-set *syntax* name transformer)))

(def get-syntax
  (lmb (name)
    (dict-get *syntax* name)))

(def macro-call?
  (lmb (form)
    (if (cons? form) (dict-has? *syntax* (head form))
        otherwise    false)))

(def macro-apply
  (lmb (form)
    (apply (get-syntax (head form)) (tail form))))

(def *eval* eval)

(def eval
  (lmb (expression environment)
    (if (macro-call? expression) (eval (macro-apply expression) environment)
        otherwise                (*eval* expression environment))))

(set-syntax 'mac
  (lmb (name args & body)
    `(set-syntax ~name (lmb ~args ~@body))))

(mac fun
  (name args & body)
  `(def ~name (lmb ~args ~@body)))

(mac let
  (formals & body)
  `((lmb ~(map head formals) ~@body) ~@(map (lmb (pair) (head (tail pair))) formals)))

