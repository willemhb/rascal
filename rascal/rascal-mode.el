;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("def" "put" "let"
    "fun" "mac" "type"
    "catch" "raise"
    "quote" "do"
    "require" "provide"
    "case" "cond" "if" "and" "or")
  '(("otherwise" . font-lock-builtin-face)
    ("\\_<resume\\_>" . font-lock-builtin-face)
    ("\\_<true\\_>" . font-lock-builtin-face)
    ("\\_<false\\_>" . font-lock-builtin-face)
    ("\\_<nul\\_>" . font-lock-builtin-face)
    ("\\_<none\\_>" . font-lock-builtin-face)
    ("\\_<not\\_>" . font-lock-builtin-face)
    ("\\_<eos\\_>" . font-lock-builtin-face)
    ("\\_<any\\_>" . font-lock-builtin-face)
    ("::" . font-lock-comment-face)
    (":[-a-zA-Z?*+0-9]+\\_>" . font-lock-builtin-face)
    ("@[-_a-zA-Z*+0-9]+" . font-lock-builtin-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
