;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("def" "put" "lmb"             ;; binding forms
    "if" "do" "quote"             ;; local control forms
    "handle" "perform" "resume"   ;; nonlocal control forms
    "c-call"                      ;; miscellaneous forms
    "fun" "mac" "var" "val"       ;; binding macros
    "struct" "union" "abstract"   ;; binding macros
    "let" "label"                 ;; binding macros
    "and" "or" "cond"             ;; branching macros
    "guard" "raise"               ;; exception macros
    "yield"                       ;; generator macros
    "async" "await"               ;; coroutine macros
    "ps" "send" "receive"         ;; process macros
    "for" "while" "in"            ;; looping macros
    "use" "do*"                   ;; misc macros
    )
  '(("\\(?:[^:]:[-a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
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
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
