;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "fun" "mac"        ;; binding syntax
    "struct" "record" "union"      ;; binding syntax (continued)
    "perform" "handle" "resume"    ;; effect syntax
    "catch" "raise" "yield"        ;; control syntax
    "if" "case" "cond" "and" "or"  ;; branching syntax
    "let" "lmb" "do" "quote"       ;; misc syntax
    "use" "c-call" "labl" "loop"   ;; misc syntax (continued)
   )
  '(("[^:]:[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ;; special constants
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\_>" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
