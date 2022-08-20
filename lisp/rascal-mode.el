;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("quote" "do" "do*" "cntl" "hndl" "with" ;; control forms
    "if" "and" "or" "cond" "case"           ;; branching forms
    "val" "fun" "mac" "type" "class"        ;; closure and binding forms
    "let" "labl"

    "where" "syntax")                       ;; non-lisp syntax
  
  '(("else" . font-lock-constant-face)
    ("otherwise" . font-lock-constant-face)
    ("error" . font-lock-warning-face)
    ("true" . font-lock-builtin-face)
    ("false" . font-lock-builtin-face)
    ("nil" . font-lock-builtin-face)
    ("none" . font-lock-builtin-face)
    (":[-+_a-zA-Z0-9?!/]+" . font-lock-builtin-face)
    ("@[-_a-zA-Z0-9?!/]+" . font-lock-builtin-face)
    ("[-_a-zA-Z0-9?!]+([-_ a-zA-Z&]*)" . font-lock-keyword-face))
  '("\\.rsp$" "\\.rdn$" "\\.rdn.o$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
