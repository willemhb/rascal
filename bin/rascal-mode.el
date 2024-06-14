;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '(
    ;; syntax affixed with `*` is superseded or otherwise not intended to be used
    ;; directly in user code
    ;; variable binding & rebinding
    "def*" "val" "var" "put"

    ;; function/macro binding
    "fun" "mac" "method" "impl"

    ;; type binding
    "type" "union" "abstract" "trait" "record" "struct"

    ;; namespace binding
    "ns" "use" "import"

    ;; closures and blocks
    "fn*" "let*" "let" "label" "loop"

    ;; threading syntax
    "->"

    ;; branching syntax
    "if" "cond" "case" "and" "or"

    ;; non-local control syntax
    "handle" "raise" "catch" "throw"

    ;; miscellaneous syntax
    "c-call" "quote" "do" "bq*" "uq*" "splc*"
    )
  '(("\\(?:[^:]:[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<nothing\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; types
    ("\\_<[A-Z][a-zA-Z0-9\\|]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("\\_<&&\\_>" . font-lock-function-name-face)
    ("\\_<&&&\\_>" . font-lock-function-name-face))
  '("\\.rl$" "\\.rls$" "\\.rlm$" "\\.rld$" "\\.rlo$") ;; generic file, source file, module file, data file, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
