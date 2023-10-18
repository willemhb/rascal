;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("fun" "mac" "var" "val" "put" ;; binding syntax
    "record" "struct" "type"      ;; binding syntax (continued)
    "abstract" "union"            ;; binding syntax (continued)
    "handle" "perform" "resume"   ;; effect syntax
    "guard" "raise" "yield"       ;; nonlocal control syntax
    "if" "and" "or"               ;; branching syntax
    "case" "cond" "not"           ;; branching syntax (continued)
    "let" "label" "loop"          ;; block syntax
    "lmb" "quote" "do"            ;; miscellaneous syntax
    "do*" "use" "c-call"          ;; miscellaneous syntax (continued)
   )
  '(("[^:]:[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("[^~]@[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ;; special constants
    ("\\_<nothing\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; types
    ("\\_<[A-Z][a-zA-Z0-9]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("\\_<&&\\_>" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\_>" . font-lock-warning-face))
  '("\\.rl$" "\\.rl.d$" "\\.rl.o$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal2 files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
