;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#")
  '("module" "fun" "mac" "type"   ;; binding syntax
    "struct" "record" "union"     ;; binding syntax (continued)
    "begin" "handle"              ;; effect syntax
    "perform" "resume"            ;; effect syntax (continued)
    "try" "catch" "finally"       ;; control syntax
    "raise" "yield"               ;; control syntax (continued)
    "async" "sync" "await"        ;; control syntax (continued)
    "if" "elif" "else" "case"     ;; branching syntax
    "cond" "and" "or" "not"       ;; branching syntax (continued)
    "stx"                         ;; branching syntax (continued)
    "do" "end"                    ;; do notation
    "loop" "use" "c-call" "quote" ;; miscellaneous syntax
   )
  '(("[^:]:[-_a-zA-Z?*+0-9/<>]+" . font-lock-type-face)
    ("@[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("::" . font-lock-comment-face)
    ("\\.\\." . font-lock-comment-face)

    ;; special constants
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)

    ;; types
    ("\\_<Any\\_>" . font-lock-type-face)
    ("\\_<None\\_>" . font-lock-type-face)
    ("\\_<Unit\\_>" . font-lock-type-face)
    ("\\_<Real\\_>" . font-lock-type-face)
    ("\\_<Small\\_>" . font-lock-type-face)
    ("\\_<Big\\_>" . font-lock-type-face)
    ("\\_<Ratio\\_>" . font-lock-type-face)
    ("\\_<Complex\\_>" . font-lock-type-face)
    ("\\_<Integer\\_>" . font-lock-type-face)
    ("\\_<Rational\\_>" . font-lock-type-face)
    ("\\_<Number\\_>" . font-lock-type-face)
    ("\\_<Boolean\\_>" . font-lock-type-face)
    ("\\_<Symbol\\_>" . font-lock-type-face)
    ("\\_<Bits\\_>" . font-lock-type-face)
    ("\\_<List\\_>" . font-lock-type-face)
    ("\\_<Tuple\\_>" . font-lock-type-face)
    ("\\_<Map\\_>" . font-lock-type-face)
    ("\\_<Function\\_>" . font-lock-type-face)
    ("\\_<Stream\\_>" . font-lock-type-face)
    ("\\_<Type\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\_>" . font-lock-warning-face))
  '("\\.rascal$" "\\.rascal.d$" "\\.rascal.o$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal2 files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
