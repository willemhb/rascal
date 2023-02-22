;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("defv" "setv"
    "fn"
    "quote" "do" "if"
    "with" "resume")
  '(("\\_<true[[:space:][:cntrl:]\\s)]" . font-lock-builtin-face)
    ("\\_<false[[:space:][:cntrl:]\\s)]" . font-lock-builtin-face)
    ("\\_<nul[[:space:][:cntrl:]\\s)]" . font-lock-builtin-face)
    ("\\_<none[[:space:][:cntrl:]\\s)]" . font-lock-builtin-face)
    ("\\_<any[[:space:][:cntrl:]\\s)]" . font-lock-builtin-face)
    ("\\_<nan[[:space:][:cntrl:]\\s)]" . font-lock-builtin-face)
    (":[-a-zA-Z?*+0-9]+[[:space:][:cntrl:]\\s)]" . font-lock-reference-face)
    ("\\_<&va[[:space:][:cntrl:]\\s)]" . font-lock-function-name-face)
    ("\\_<&body[[:space:][:cntrl:]\\s)]" . font-lock-function-name-face)
    ("\\_<&opt[[:space:][:cntrl:]\\s)]" . font-lock-function-name-face)
    ("\\_<&kw[[:space:][:cntrl:]\\s)]" . font-lock-function-name-face)
    ("\\_<&cc[[:space:][:cntrl:]\\s)]" . font-lock-function-name-face)
    ("\\_<error[[:space:][:cntrl:]\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
