;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-no-lisp-mode
  '("#" ("#|" . "|#"))
  '( ;; special forms
    "quote" "def" "put" "fn" "def-stx"
    ;; macros
    "fun" "mac" "var" "val" "type" "trait"
    "let" "label"
    "if" "elif" "else" "cond"
    "match" "case" "when" "stx"
    "and" "or" "not"
    "try" "catch" "throw" "except" "finally"
    "raise" "abort" "handle"
    "import" "use" "expose" "module"
    "receive" "send" "await"
    "do" "end")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/<>=]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
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
    ("\\_<&\\_>" . font-lock-function-name-face))
  '("\\.rascal$")
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-no-lisp-mode)
;;; rascal-mode.el ends here
