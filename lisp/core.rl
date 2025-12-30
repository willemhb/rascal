
;; more sophisticaated functions and macros that benefit from a working backquote.
(stx when
 (test & body)
 `(if ~test
   (do ~@body)))

(stx unless
 (test & body)
  `(if (not ~test)
    (do ~@body)))

;; block and looping forms
(stx let
 ;; 
 (binds & body)
  (fun xform-bind (bind) `(def ~@bind))
   (def def-forms (map xform-bind binds))
   `((fn () ~@def-forms ~@body)))

 (stx label
  ;; standard looping syntax.
  ;; defines a recursive function inside
  ;; a thunk, invokes the body inside the thunk,
  ;; and immediately invokes the thunk.
  ;; this really calls for actually implementing tail recursion.
  (name inits & body)
  (def formals (fmap fst inits))
  (def values  (fmap snd inits))
  (def defn-form
  `(def ~name (fn ~formals ~@body)))
  `((fn () ~defn-form (~name ~@inits))))

; ;(fun macro-call?
;  (form)
;  (and (list? form)
;       (sym? (head form))
;       (macro-name? (head form))))

; (fun macroexpand-1
;  (form)
;  (if (macro-call? form)
;       (macro-apply (head form) form (*environ*) (tail form))
;      otherwise form))

(stx -> ;; just for fun
 (init form & forms)
 (fun expand-form
  (init form)
  (if (list? form)
   `(~(head form) ~init ~@(tail form))
   `(~form ~init)))
 (if (empty? forms)
  (expand-form init form)
  `(-> ~(expand-form init form) ~@forms)))

;; other useful methods
(fun some?
 (p? xs)
 (if (empty? xs)
   false
   (p? (head xs))
    true
   otherwise
    (some? p? (tail xs))))

(fun every?
 (p? xs)
 (if (empty? xs)  true
   (p? (head xs)) (every? p? (tail xs))
   otherwise      false))

(fun fmap
 (f & xss)
 (if (some? empty? xss)
  ()
  (cons (apply f (fmap head xss))
        (map f (fmap tail xss)))))
