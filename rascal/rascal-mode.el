;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("def" "put" "let"
    "fun" "mac" "type" "with"
    "if" "cond" "case" "and" "or"
    "quote" "do" "do*"
    "require" "provide")
  '(("otherwise" . font-lock-builtin-face)
    ("resume\\W" . font-lock-builtin-face)
    ("true\\W" . font-lock-builtin-face)
    ("false\\W" . font-lock-builtin-face)
    ("nul\\W" . font-lock-builtin-face)
    ("none\\W" . font-lock-builtin-face)
    ("\\Wnot\\W" . font-lock-builtin-face)
    ("eos\\W" . font-lock-builtin-face)
    ("any\\W" . font-lock-builtin-face)
    ("[^a-zA-Z?*+]:[-a-zA-Z?*+0-9]+" . font-lock-builtin-face)
    ("@[-_a-zA-Z?*+0-9]+" . font-lock-builtin-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
