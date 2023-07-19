;; naive macro implementation
(def *syntax-table* (table))

(def get-syntax
  (lmb (name)
    (get *syntax-table* name :fallback nul)))

(def set-syntax
  (lmb (name xform)
    (set *syntax-table* name xform)))

(def macro-call?
  (lmb (form)
    (if (cons? form)
        (has? *syntax-table* (head form)))))

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
(def %compile% compile)

(def eval
  (lmb (x)
    (%eval% (macro-expand* x))))

(def compile
  (lmb (x)
    (%compile% (macro-expand* x))))

;; basic binding macros
(set-syntax 'mac
  (lmb (name args & body)
    `(set-syntax ~name (lmb ~args ~@body))))

(set-syntax 'fun
  (lmb (name args & body)
    `(def ~name (lmb ~args ~@body))))

;; use implementation
(def *modules*  (table))

(def use
  (lmb (module)
    (def expanded (expand-file-name module))
    (if (has? *modules* expanded)
        (get *modules* expanded)
        (do (def loaded (load expanded))
            (set *modules* expanded loaded)
            loaded))))