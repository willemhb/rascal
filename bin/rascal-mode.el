;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '( ;; special forms
    "quote" "def" "def-stx" "def-multi" "def-method" "put" "if" "and" "or" "do" "fn" "c-call"
    ;; macros
    "fun" "stx" "let" "cond" "case")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; builtins
    ("\\_<not\\_>" . font-lock-builtin-face)
    ("\\_<typeof\\_>" . font-lock-builtin-face)
    ("\\_<apply\\_>" . font-lock-builtin-face)
    ("\\_<load\\_>" . font-lock-builtin-face)
    ("\\_<error\\_>" . font-lock-warning-face)
    ;; types
    ("\\_<[A-Z][a-zA-Z0-9\\|]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face))
  '("\\.rl$")
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
