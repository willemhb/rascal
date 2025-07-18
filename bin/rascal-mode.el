;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#")
  '(;; actual syntax (near-term plan to support)
    ;; binding/declaration syntax
    "var" "val" "fun" "mac" "type" "mut" "as" "with"
    ;; conditional syntax
    "if" "else" "cond" "otherwise" "and" "or" "not"
    ;; block/loop syntax
    "label" "loop" "do" "end"
    ;; module syntax
    "require" "provide"
    ;; control syntax
    "control" "raise" "from" "when" "handle" "begin"
    ;; miscellaneous syntax
    "quote")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/<>=.]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/<>=.]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/.<>=]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/.<>=]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<nan\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; builtins
    ("\\_<print\\_>" . font-lock-builtin-face)
    ("\\_<error\\_>" . font-lock-warning-face)
    ;; types
    ("\\_<[A-Z][-_a-zA-Z0-9.*\\|]*\\_>" . font-lock-type-face))
  '("\\.rl$")
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
