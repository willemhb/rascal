;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "fun" "mac" "var" "record" "struct" "union" ;; binding forms & macros
    "handle" "perform" "resume" "try" "catch" "raise"       ;; nonlocal control forms & macros
    "do" "do*" "quote" "backquote" "unquote" "splice"       ;; control forms & macros
    "if" "case" "cond" "and" "or"                           ;; branching forms & macros
    "let" "label" "syntax"                                  ;; closure/block forms & macros
    "module" "require" "provide" "c-call" "java-call"       ;; misc forms & macros
   )
  '(("\\(:[-a-zA-Z?*+0-9]+\\)[[:space:][:cntrl:],\\s)]" 1 font-lock-reference-face)
    ;; immediate types
    ("\\_<real[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<fixnum[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<small[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<glyph[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<bool[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<primitive[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<unit[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<port[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<native[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<pointer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<c-string[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; user object types
    ("\\_<symbol[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<cons[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<tuple[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<vector[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<dict[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<set[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<record[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<struct[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<binary[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<function[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<module[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; internal object types (node types for user objects)
    ("\\_<vector-node[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<vector-leaf[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<dict-node[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<dict-leaf[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<set-node[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<set-leaf[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; numeric object types
    ("\\_<bigint[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<ratio[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<complex[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
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
    ("\\_<variable[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<upvalue[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; type system types
    ("\\_<type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<data-type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<union-type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<any[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<none[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; misc builtin types
    ("\\_<list[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ;; special constants
    ("\\_<nul[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ;; syntax
    ("\\_<otherwise[[:space:][:cntrl:],\\s)]" . font-lock-function-name-face)
    ("\\_<&[[:space:][:cntrl:],\\s)]" . font-lock-function-name-face)
    ;; misc
    ("\\_<error[[:space:][:cntrl:],\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
