;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("fun" "mac" "type"              ;; binding syntax
    "abstract" "record"             ;; binding syntax (continued)
    "struct" "union"                ;; binding syntax (continued)
    "val" "var" "module"            ;; binding syntax (continued)
    "handle" "perform" "resume"     ;; effect syntax
    "guard" "raise" "finally"       ;; exception syntax
    "yield" "async" "sync" "await"  ;; concurrency syntax
    "if" "and" "or" "not"           ;; branching syntax
    "case" "cond" "stx"             ;; branching syntax (continued)
    "let" "label" "loop"            ;; block syntax
    "quote" "backquote"             ;; quote syntax
    "unquote" "splice"              ;; quote syntax (continued)
    "use" "c-call" "lmb"            ;; miscellaneous syntax
    "program" "do" "do*"            ;; miscellaneous syntax (continued)
   )
  '(("[^:]:[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("@[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("::" . font-lock-comment-face)
    ("\\.\\." . font-lock-comment-face)
    ;; special constants
    ("\\_<nothing\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; types
    ("\\_<Any\\_>" . font-lock-type-face)
    ("\\_<None\\_>" . font-lock-type-face)
    ("\\_<Unit\\_>" . font-lock-type-face)
    ("\\_<Term\\_>" . font-lock-type-face)
    ("\\_<Float\\_>" . font-lock-type-face)
    ("\\_<Small\\_>" . font-lock-type-face)
    ("\\_<Boolean\\_>" . font-lock-type-face)
    ("\\_<Glyph\\_>" . font-lock-type-face)
    ("\\_<Big\\_>" . font-lock-type-face)
    ("\\_<Symbol\\_>" . font-lock-type-face)
    ("\\_<Function\\_>" . font-lock-type-face)
    ("\\_<Stream\\_>" . font-lock-type-face)
    ("\\_<List\\_>" . font-lock-type-face)
    ("\\_<Cons\\_>" . font-lock-type-face)
    ("\\_<Vector\\_>" . font-lock-type-face)
    ("\\_<Map\\_>" . font-lock-type-face)
    ("\\_<Bits\\_>" . font-lock-type-face)
    ("\\_<Type\\_>" . font-lock-type-face)
    ("\\_<Struct\\_>" . font-lock-type-face)
    ("\\_<Record\\_>" . font-lock-type-face)
    ("\\_<Ratio\\_>" . font-lock-type-face)
    ("\\_<Complex\\_>" . font-lock-type-face)
    ("\\_<Integer\\_>" . font-lock-type-face)
    ("\\_<Rational\\_>" . font-lock-type-face)
    ("\\_<Real\\_>" . font-lock-type-face)
    ("\\_<Number\\_>" . font-lock-type-face)
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
