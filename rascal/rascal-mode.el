;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("fun" "mac" "cntl" "syntax"
    "type" "data" "union" "class"
    "val" "var" "do" "quote"
    "cond" "if" "and" "or" "case"
    "let"  "with")
  '(("otherwise" . font-lock-builtin-face)
    ("resume" . font-lock-builtin-face)
    ("escape" . font-lock-builtin-face)
    ("abort" . font-lock-builtin-face)
    ("true" . font-lock-builtin-face)
    ("false" . font-lock-builtin-face)
    ("nil" . font-lock-builtin-face)
    ("none" . font-lock-builtin-face)
    ("\\W:\\{1\\}[-+_a-zA-Z0-9?!/]+" . font-lock-builtin-face)
    ("[A-Z][_a-zA-Z0-9]*" . font-lock-function-name-face))
  '("\\.rsp$" "\\.rdn$" "\\.rdn.o$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
