\;; begin syntax.rl

(def *syntax* {})
(def *eval* eval)
(def *compile* compile)

(def set-syntax
  (lmb (name xform)
    (put *syntax* name xform)))

(def get-xform
  (lmb (form)
    (*syntax* (hd form))))

(def xform
  (lmb (form)
    (apply (get-xform form) (tl form))))

(def macro-call?
  (lmb (form)
    (and (cons? form)
         (sym?  (hd form))
         (elem? (hd form) *syntax*))))

(def macro-expand-1
  (lmb (form)
    (cond
      ((atom? form) form)
      ((macro-call? form) (xform form))
      (:otherwise form))))

(def macro-expand
  (lmb (form)
    (cond
      ((atom? form) form)
      ((macro-call? form) (macro-expand (xform form)))
      (:otherwise (map macro-expand form)))))

(def eval
  (lmb (x)
    (*eval* (macro-expand x))))

(def compile
  (lmb (x)
    (*compile* (macro-expand x))))

(set-syntax 'defm
            (lmb (name args & body)
              (list 'set-syntax
                    (list 'quote name)
                    (cons* 'lmb args body))))

(defm defn
  (name args & body)
  (list 'def
        name
        (cons* 'lmb args body)))

;; end syntax.rl
