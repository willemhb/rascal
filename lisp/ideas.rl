;; right now this file is just for brainstorming desired syntax and whatnot. The code here
;; doesn't run and isn't intended to.
(def-stx mac
  (fn (&form &env name args & body)
    `(def-stx ~name
      (fn (&form &env ~args) ~@body))))

(mac thunk
  (xpr) `(fn () ~xpr))

(mac catch
  "Wrapper for slightly less readable builtin 'catch' form."
  (h & body)
  `(handle (fn ~@h)
    ~@body))

(val &loaded (table))

(mac require
  "Idempotent load (don't load a file twice)."
  (fname)
  (if (has? &loaded fname)
    `(quote ~(get &loaded fname))
    `(let (#result (load ~fname))
       (tset &loaded ~fname #result))))

;; desired syntax for except is that it provides a case-like interface for handling
;; different types of exceptions (in fact probably to be implemented by wrapping the given
;; handler in a case form). Note that the simple implementation above does *not* meet this
;; goal.

(except
  ((e m)
     :type-error    ...
     :zero-division ...
     otherwise      ...)
  (print "Hello.")
  (print "Hello?")
  (print ">:(")
  (/ 2 0))
