;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "fun" "mac" "type" ;; binding syntax
    "ccc" "catch" "throw"          ;; control syntax
    "if" "case" "cond" "and" "or"  ;; branching syntax
    "let" "lmb" "do" "quote"       ;; misc syntax
    "require" "c-call"             ;; misc syntax (continued)
   )
  '(("[^:]\\(:[-a-zA-Z?*+0-9/]+\\)\\b" 1 font-lock-reference-face)
    ;; special 
    ("\\bany\\b" . font-lock-builtin-face)
    ("\\bunit\\b" . font-lock-builtin-face)
    ("\\bnone\\b" . font-lock-builtin-face)
    ;; special constants
    ("\\bnul\\b" . font-lock-constant-face)
    ("\\btrue\\b" . font-lock-constant-face)
    ("\\bfalse\\b" . font-lock-constant-face)
    ;; syntax
    ("\\botherwise\\b" . font-lock-function-name-face)
    ("&" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\b" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
