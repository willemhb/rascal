;; speculative lisp syntax.

(load "boot.rl")

(provide Core)

@doc "Rascal core library (basic syntax, not including the really primitive stuff)."

;; example of simple function definitions (demonstrates multimethod syntax).
(fun count
  @doc "Return the number of elements in `xs`."
  (xs))

(fun count
  ((xs Vec))
  (vec-count xs))

(fun count
  ((xs Str))
  (str-count xs))

(fun count
  ((xs Bin))
  (bin-count xs))

(fun count
  ((xs Map))
  (map-count xs))

(fun empty?
  @doc "Return `true` if `xs` is an empty sequence."
  (xs)
  (= (count xs) 0))

;; examples of algebraic effect usage.
(fun with-output-stream
  @doc "Call `f` on a fresh stream for file `s`. Clean up the file handle even if an error occurs. Raise an error if no such file exists."
  ((s Str) (f Fun))
  (if (not (file-exists? s))
    (raise :error "file not found.")
    (begin
      (var p (open s))
      (f p)
      (handle k
        ([:return x] ; normal exit
           (close p) x)
        ([:resume x]
           (put p (open s)) (k x))
        ([:error m]
           (close p) (prn m) nul)
        ([op & args]
           (close p) (raise* k op & args))))))