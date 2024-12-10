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
    ;; binding syntax
    "def*" "put*" "fn*" "fun" "mac" "val" "var" "put" "type"

    ;; module system
    "ns" "use"

    ;; closures and blocks
    "let" "label"

    ;; threading syntax
    "->"

    ;; branching syntax
    "if*" "if" "cond" "case" "and" "or"

    ;; non-local control syntax
    "hndl*" "raise*" "hndl" "raise" "catch" "throw"

    ;; miscellaneous syntax
    "c-call" "quote" "do" "bq*" "uq*" "splc*"
    )
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
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
