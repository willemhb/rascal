;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("var" "val" "put" "fn" "mac" "type"
    "lmb" "let" "labl"
    "quote" "do" "do*"
    "if" "cond" "case" "and" "or"
    "with" "resume" "catch" "raise"
    "require" "c-call")
  '(("::" . font-lock-comment-face)
    ("&" . font-lock-function-name-face)
    ("\\_<real[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<fixnum[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<int[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<glyph[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<bool[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<ratio[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<bigint[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<ratio[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<complex[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<symbol[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<stream[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<function[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<string[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<binary[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<list[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<vector[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<tuple[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<dict[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<set[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<record[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<upvalue[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<chunk[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<dispatch[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<method[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<pointer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<none[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<any[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<unit[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<nul[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<nan[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    (":[-a-zA-Z?*+0-9]+[[:space:][:cntrl:],\\s)]" . font-lock-reference-face)
    ("\\_<error[[:space:][:cntrl:],\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
