;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "lmb" "if" "quote" "do" "ccc" ;; special forms
    "fun" "mac" "var"                         ;; binding macros
    "case" "cond" "and" "or"                  ;; branching macros
    "let" "labl"                              ;; closure macros
   )
  '(("\\(:[-a-zA-Z?*+0-9]+\\)[[:space:][:cntrl:],\\s)]" 1 font-lock-reference-face)
    ("\\_<number[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<fixnum[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<glyph[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<bool[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<port[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<native[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<pointer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<unit[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<symbol[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<list[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<vector[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<table[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<binary[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<ns[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<envt[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<chunk[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<closure[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<control[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<nil[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<otherwise[[:space:][:cntrl:],\\s)]" . font-lock-function-name-face)
    ("\\_<&[[:space:][:cntrl:],\\s)]" . font-lock-function-name-face)
    ("\\_<error[[:space:][:cntrl:],\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
