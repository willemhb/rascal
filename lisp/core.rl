#| functions and macros for bootstrapping the language.
   mostly very simple core utilities, core syntax, and helpers for syntactic analysis.

 |#

;; binding macros.
(mac let
  ((List formals

;; exceptions.

;; module system macros.
(val &used (MutDict))

(mac use
  "Load files just once."
  ((List files))
  (for (file files)
    (unless (has? &used file)
      (assoc! &used file (load file)))
    (ref &used file)))

