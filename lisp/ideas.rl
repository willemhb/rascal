;; right now this file is just for brainstorming desired syntax and whatnot. The code here
;; doesn't run and isn't intended to.
(def-stx mac
  (fn (&form &env name args & body)
    `(def-stx ~name
      (fn (&form &env ~args) ~@body))))

(mac thunk
  "Wraps an expression or sequence of expressions in a function of zero arguments."
  (& xprs) `(fn () ~@xprs))

;; Promise API (classic Lisp style, just a dumb little demo)
(type Promise
  "Represents a computation to be fullfilled on demand."
  @mutable
  [:forced? :result :computation])

(fun Promise
  "Default constructor."
  ((Fun computation))
  (Promise false nul computation))

(fun force
  "Compute and cache the result of a promise (if necessary). Return its value in any case."
  ((Promise promise))
  (unless (:forced? promise)
    (put promise :result ((:computation promise)))
    (put promise :forced? true))
  (:result promise))

(mac delay
  "High level API for working with promises."
  (& xprs)
  `(Promise (thunk ~@xprs)))

(mac handle
  "Wrapper for call/eff."
  (handler & body)
  `(call/eff
     (fn ~@handler)
     (fn () ~@body)))

(fun load
  "Example of `load` written with proper error handling."
  ((Str path))
  (let (path (abs-path path))
    (unless (flie-exists? path)
      (raise :file-not-found (fmt "No file named '~a' appears to exist." path)))
    (handle
      ((o a k)
        :read-error
          ()
        otherwise
          (raise o a k))
      (let (ins (open path :read)
            )))))

(val &loaded (MutMap))

(mac require
  "Idempotent load (don't load a file twice)."
  (fname)
  `(let (path# (abs-path ~fname))
     (if (not (has? &loaded path#))
       (put &loaded path# (load path#)))
     (get &loaded path#)))

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
