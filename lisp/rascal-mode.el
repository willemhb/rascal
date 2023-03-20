;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("var" "put" "lmb" "do" "quote" "if"
    "val" "mac" "fn" "type" "let" "labl"
    "cond" "case" "and" "or" "do*"
    "with" "resume"
    "require" "provide")
  '((":[-a-zA-Z?*+0-9]+[[:space:][:cntrl:],\\s)]" . font-lock-reference-face)
    ("\\_<primitive-type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<object-type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<union-type[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<array-node[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<array-leaf[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<map-node[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<map-leaf[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<map-leaves[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<method-table[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<method[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<chunk[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<closure[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<variable[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<namespace[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<environment[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<control[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<symbol[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<function[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<port[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<binary[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<string[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<tuple[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<list[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<vector[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<dict[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<set[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<table[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<alist[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<buffer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<struct[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<record[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<complex[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<ratio[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<big[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<sint-8[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<uint-8[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<sint-16[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<uint-16[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<sint-32[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<uint-32[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<real-32[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<fixnum[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<real[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<ascii[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<latin-1[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<utf-8[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<utf-16[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<utf-32[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<boolean[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<pointer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<none[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<unit[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<any[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("&" . font-lock-function-name-face)
    ("\\_<error[[:space:][:cntrl:],\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
