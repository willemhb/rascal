;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '(;; binding forms
    "def*" "fn*" "put"
    ;; local control forms
    "if" "do" "quote"
    ;; nonlocal control forms (effects system)
    "handle" "perform" "enter" "exit"
    ;; miscellaneous forms
    "c-call"
    ;; binding macros
    "var" "val"
    "fun" "fun*" "mac" "mac*"
    "struct" "union" "abstract"
    "let" "label"
    ;; branching macros
    "and" "or" "cond" "case"
    ;; exception handling macros
    "guard" "raise"
    ;; generator macros
    "generator" "yield"
    ;; async macros
    "async" "sync" "await"
    ;; actor macros
    "ps" "send" "receive"
    ;; miscellaneous macros
    "use" "with" "do*")
  '(("\\(?:[^:]:[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<nothing[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<nul[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<true[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ("\\_<false[[:space:][:cntrl:],\\s)]" . font-lock-constant-face)
    ;; types
    ("\\_<[A-Z][a-zA-Z0-9]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("\\_<&&\\_>" . font-lock-function-name-face))
  '("\\.rl$" "\\.rl.d$" "\\.rl.o$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
