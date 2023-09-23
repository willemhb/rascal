;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#")
  '("module" "fun" "mac" "type"   ;; binding syntax
    "struct" "record" "union"     ;; binding syntax (continued)
    "perform" "handle" "resume"   ;; effect syntax
    "try" "catch" "raise" "yield" ;; control syntax
    "if" "elif" "else" "case"     ;; branching syntax
    "cond" "and" "or" "not"       ;; branching syntax (continued)
    "do" "end"                    ;; do notation
    "use" "c-call" "quote"        ;; misc syntax
   )
  '(("[^:]:[-_a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("@[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("::" . font-lock-comment-face)
    ;; special constants
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\_>" . font-lock-warning-face))
  '("\\.rascal$" "\\.rascal.d$" "\\.rascal.o$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal2 files.")

(provide 'rascal2-mode)
;;; rascal2-mode.el ends here
