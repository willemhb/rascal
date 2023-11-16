#| functions and macros for bootstrapping the language.
   mostly very simple core utilities, core syntax, and helpers for syntactic analysis.

 |#

;; binding macros.
(mac let
  "Evaluate name/binding pairs in formals sequentially, then evaluate body in a context with those bindings."
  ((List formals) & body)
  `((fn* () ~@(map formals #`(val ~(hd %) ~(hd|tl %))) ~@body)))

;; looping macros.
(mac for
  "Loop over elements of a collection."
  ((List bindings) & body)
  (let ((inits   (template ((#producer (seq %2)) (%1 (fst #producer))) bindings))
        (rebinds (template )))
    `()))

;; exceptions.

;; module system macros.
(val &used (MutDict))

(mac use
  "Load files just once."
  ((List files))
  (for ((file files))
    (var fname (if (string? file)
                   file
                   (as-file-name file)))
    (unless (has? &used fname)
      (assoc! &used fname (load fname)))
    (ref &used fname)))
