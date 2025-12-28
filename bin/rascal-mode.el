;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '( ;; special forms
    "quote" "def" "def-stx" "def-multi" "def-method"
    "put" "if" "and" "or" "do" "fn"
    "control" "raise"
    ;; macros
    "fun" "stx" "fun*" "stx*" "let" "cond" "case"
    "label" "loop" "label*" "handle" "require" "provide")
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
    ("\\_<eval\\_>" . font-lock-builtin-face)
    ("\\_<print\\_>" . font-lock-builtin-face)
    ("\\_<read\\_>" . font-lock-builtin-face)
    ("\\_<ffi-call\\_>" . font-lock-builtin-face)
    ("\\_<ffi-open\\_>" . font-lock-builtin-face)
    ("\\_<ffi-close\\_>" . font-lock-builtin-face)
    ("\\_<ffi-sym\\_>" . font-lock-builtin-face)
    ("\\_<list\\_>" . font-lock-builtin-face)
    ("\\_<sym\\_>" . font-lock-builtin-face)
    ("\\_<tuple\\_>" . font-lock-builtin-face)
    ("\\_<str\\_>" . font-lock-builtin-face)
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
