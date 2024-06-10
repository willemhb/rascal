;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '(
    ;; special forms (well-defined but rigid syntax, intended as building blocks)
    ;; general binding forms (defvar, putvar)
    "defvar*" "putvar*"

    ;; function binding forms (defgeneric, defmethod).
    "defgeneric*" "defun*" "defmacro*" "defmethod*"

    ;; type binding forms (deftype, defabstract, defunion, defstruct, defrecord, deftrait).
    "deftype*" "defabstract*" "defunion*" "defstruct*" "defrecord*" "deftrait*"

    ;; miscellaneous binding forms (lambda)
    "fn*"

    ;; namespace special forms (namespace)
    "ns*" "use*"

    ;; non-local control forms (delimited continuations)
    "prompt*" "cntl*"

    ;; miscellaneous special forms.
     "ffi*"

     ;; base special forms (not discouraged in user code).
     "if" "quote" "do"

    ;; binding syntax.
     "val" "var" "put" "fun" "mac" "type" "impl"

    ;; block and looping syntax.
    "let" "label" "loop" "for" "while"

    ;; branching syntax.
    "cond" "case" "and" "or"

    ;; advanced control syntax.
    "handle" "raise" "resume" "catch" "throw" "yield"

    ;; package syntax
    "module" "import" "export" "begin"

    ;; miscellaneous syntax.
    "c-call" "bq*" "uq*" "splc*"
    )
  '(("\\(?:[^:]:[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<nothing[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<nul[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ;; types
    ("\\_<[A-Z][a-zA-Z0-9]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("\\_<&&\\_>" . font-lock-function-name-face))
  '("\\.rl$" "\\.rls$" "\\.rlm$" "\\.rld$" "\\.rlo$") ;; generic file, source file, module file, data file, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
