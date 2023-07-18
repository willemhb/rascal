;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "fun" "mac" "type" ;; binding syntax
    "ccc" "try" "catch"            ;; control syntax
    "if" "case" "cond" "and" "or"  ;; branching syntax
    "let" "lmb" "do" "quote"       ;; misc syntax
   )
  '(("\\(:[-a-zA-Z?*+0-9/]+\\)[[:space:][:cntrl:],\\s)]" 1 font-lock-reference-face)
    ;; immediate types
    ("\\_<real[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<glyph[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<port[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<native[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<pointer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; user object types
    ("\\_<symbol[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<function[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<list[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<record[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; utility object types
    ("\\_<alist[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<table[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<buffer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; interpreter object types
    ("\\_<chunk[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<closure[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<control[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<method-table[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<method-table-node[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<method[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; type system types
    ("\\_<type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<any[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<unit[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<none[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; special constants
    ("\\_<nul[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ;; syntax
    ("\\_<otherwise[[:space:][:cntrl:],\\s)]" . font-lock-function-name-face)
    ("\\_<&?&[[:space:][:cntrl:],\\s)]" . font-lock-function-name-face)
    ;; misc
    ("\\_<error[[:space:][:cntrl:],\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
