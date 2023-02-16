;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("defv" "defc" "lmb"
    "defn" "defm"
    "do" "if" "and" "or" "cond" "case"
    "putv" "quote"
    "raise" "hndl")
  '(("\\_<true\\_>" . font-lock-builtin-face)
    ("\\_<false\\_>" . font-lock-builtin-face)
    ("\\_<nul\\_>" . font-lock-builtin-face)
    (":[-a-zA-Z?*+0-9]+\\_>" . font-lock-builtin-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
