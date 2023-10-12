;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("fun" "mac" "var"            ;; binding syntax
    "record" "union"             ;; binding syntax (continued)
    "handle" "perform" "resume"  ;; effect syntax
    "guard" "raise" "finally"    ;; exception syntax
    "if" "and" "or" "case"       ;; branching syntax
    "let" "with" "label" "loop"  ;; block syntax
    "quote" "lmb" "use" "c-call" ;; miscellaneous syntax
    "annot" "do"                 ;; miscellaneous syntax (continued)
   )
  '(("[^:]:[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("@[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ;; special constants
    ("\\_<nothing\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; user types
    ("\\_<Unit\\_>" . font-lock-type-face)
    ("\\_<Number\\_>" . font-lock-type-face)
    ("\\_<Boolean\\_>" . font-lock-type-face)
    ("\\_<Glyph\\_>" . font-lock-type-face)
    ("\\_<Symbol\\_>" . font-lock-type-face)
    ("\\_<Function\\_>" . font-lock-type-face)
    ("\\_<Stream\\_>" . font-lock-type-face)
    ("\\_<List\\_>" . font-lock-type-face)
    ("\\_<Vector\\_>" . font-lock-type-face)
    ("\\_<Map\\_>" . font-lock-type-face)
    ("\\_<Bits\\_>" . font-lock-type-face)
    ;; fucked up types
    ("\\_<Type\\_>" . font-lock-type-face)
    ("\\_<Record\\_>" . font-lock-type-face)
    ("\\_<Union\\_>" . font-lock-type-face)
    ("\\_<Any\\_>" . font-lock-type-face)
    ("\\_<None\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("\\_<&&\\_>" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\_>" . font-lock-warning-face))
  '("\\.rl$" "\\.rl.d$" "\\.rl.o$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal2 files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
