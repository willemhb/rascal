;; speculative lisp syntax.

(load "boot.rl")

(module Base
  @doc "Notional Rascal base library (Lisp variant)."

  (fun count
    [xs]
    (raise (error/method "No method matching signature (#{xs.type}).")))

  (fun count
    [xs: List]
    xs.list-count)

  (fun count
    [xs: Map]
    xs.map-count)

  (fun with-output-file
    [path: Str func: Fun]
    (if (not|file-exists? path)
      (raise (error/file-not-found "File '#{path}' does not exist."))
      (control continue
          (var output-file)
        (handle (enter)
          (put output-file (open path)))
        (handle (exit)
          (close path)
          (put output-file nul))
        (handle (error/* message) nul)
        (begin )))))