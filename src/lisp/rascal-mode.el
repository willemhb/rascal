;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "var" "val" "fn" "mac" "type"   ;; binding/assignment forms & macros
    "lmb" "let" "labl"                          ;; closure forms & macros
    "for" "while" "loop"                        ;; looping macros
    "require" "provide" "lib"                   ;; import/export/library macros
    "if" "case" "cond" "and" "or"               ;; branching forms & macros
    "prompt" "cntl" "with"                      ;; continuation handling forms & macros
    "try" "catch" "throw"                       ;; exception handling macros
    "async" "sync" "await" "yield"              ;; coroutine macros
    "quote" "do" "do*")                         ;; misc control macros
  '(("::" . font-lock-comment-face)
    (":[-a-zA-Z?*+0-9]+[[:space:][:cntrl:],\\s)]" . font-lock-reference-face)
    ("\\_<symbol[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<function[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<port[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<list[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<chunk[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<closure[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<control[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<table[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<alist[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<buffer[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<sint-32[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<fixnum[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<real[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<boolean[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<unit[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<any[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<none[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<data[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<union[[:space:][:cntrl:],\\s)]" . font-lock-builtin-face)
    ("\\_<nul[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<error[[:space:][:cntrl:],\\s)]" . font-lock-warning-face))
  '("\\.rl$" "\\.rdn$" "\\.rlo$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
