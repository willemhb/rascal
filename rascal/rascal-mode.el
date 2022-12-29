;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("def" "put" "let" "fun"
    "catch" "cond" "case" "quote" "do"
    "type" "mac" "with" "and" "or")
  '(("otherwise" . font-lock-builtin-face)
    ("\\Wresume\\W" . font-lock-builtin-face)
    ("true" . font-lock-builtin-face)
    ("false" . font-lock-builtin-face)
    ("nul" . font-lock-builtin-face)
    ("\\Wnone\\W" . font-lock-builtin-face)
    ("\\Wnot\\W" . font-lock-builtin-face)
    ("\\Weos\\W" . font-lock-builtin-face)
    ("\\Wany\\W" . font-lock-builtin-face)
    ("[^a-zA-Z?*+]:[-a-zA-Z?*+0-9]+" . font-lock-builtin-face)
    ("@[-_a-zA-Z*+0-9]+" . font-lock-builtin-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
