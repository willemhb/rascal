;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#")
  '("fun" "val" "do" "if"  "elif"
    "else" "and" "or")
  '(("otherwise" . font-lock-builtin-face)
    ("true" . font-lock-builtin-face)
    ("false" . font-lock-builtin-face)
    ("nil" . font-lock-builtin-face)
    ("none" . font-lock-builtin-face)
    (":\\{1\\}[-+_a-zA-Z0-9?!/]+" . font-lock-builtin-face)
    ("@[-_a-zA-Z0-9?!/]+" . font-lock-builtin-face))
  '("\\.rsp$" "\\.rdn$" "\\.rdn.o$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
