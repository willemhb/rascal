
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
  (def formals (map fst inits))
  (def values  (map snd inits))
  (def defn-form
  `(def ~name (fn ~formals ~@body)))
  `((fn () ~defn-form (~name ~@inits))))

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
 (if (empty? xs)
   true
   (p? (head xs))
    (every? p? (tail xs))
   otherwise
    false))

(fun map
 (f & xss)
 (if (some? empty? xss)
  ()
  (cons (apply f (map head xss))
        (map f (map tail xss)))))

