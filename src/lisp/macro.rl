;; naive macro implementation
(def *syntax-table* (table))

(def get-syntax
  (lmb (name)
    (table-get *syntax-table* name :fallback nul)))

(def set-syntax
  (lmb (name xform)
    (table-set *syntax-table* name xform)))

(def macro-call?
  (lmb (form)
    (if (cons? form)
        (table-has? *syntax-table* (head form)))))

(def macro-expand
  (lmb (form)
    (if (macro-call? form)
        (do (def macro (head form))
            (def body  (tail form))
            (def xform (get-syntax macro))
            (apply xform body))
        form)))

(def macro-expand*
  (lmb (form)
    (if (macro-call? form)
        (do (def macro (head form))
            (def body  (tail form))
            (def xform (get-syntax macro))
            (def xpand (apply xform body))
            (macro-expand* xpand))
        (if (cons? form)
            (macro-expand* (map macro-expand* form))
            form))))

(def %eval% eval)

(def eval
  (lmb (x e)
    (%eval% (macro-expand* x) e)))
