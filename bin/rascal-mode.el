;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '(
    ;; binding syntax.
    ;; base (mostly used only in bootstrapping).
    "def*" "put*" "spec*" "lmb*"

    "fun"  "mac" "type" "val" "var"

    ;; block and looping syntax.
    "let" "label" "loop" "for" "while"

    ;; branching syntax.
    "if" "cond" "case" "and" "or"

    ;; advanced control syntax.
    "handle" "raise" "abort" "catch" "throw"

    ;; package syntax.
    "module" "import" "export" "begin"

    ;; miscellaneous syntax.
    "quote" "do" "c-call"
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
