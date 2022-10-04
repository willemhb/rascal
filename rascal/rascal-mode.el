;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '("var" "let" "with"     ;; binding
    "fun" "mac" "cntl"     ;; closures
    "type" "data" "union"  ;; types
    "cond" "if" "and" "or" ;; branching
    "import" "export"      ;; module system
    "do" "quote" "c-call") ;; misc
  '(("otherwise" . font-lock-builtin-face)
    ("resume\\W" . font-lock-builtin-face)
    ("escape\\W" . font-lock-builtin-face)
    ("abort\\W" . font-lock-builtin-face)
    ("true\\W" . font-lock-builtin-face)
    ("false\\W" . font-lock-builtin-face)
    ("nul\\W" . font-lock-builtin-face)
    ("none\\W" . font-lock-builtin-face)
    ("\\Wnot\\W" . font-lock-builtin-face)
    ("eos\\W" . font-lock-builtin-face)
    ("any\\W" . font-lock-builtin-face)
    ("[^a-zA-Z?*+]:[-a-zA-Z?*+]+" . font-lock-builtin-face)
    ("[A-Z][_a-zA-Z0-9]*" . font-lock-function-name-face))
  '("\\.rsp$" "\\.rdn$" "\\.rdn.o$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
