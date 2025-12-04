;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#" ("\"\"\"" . "\"\"\""))
  '(;; actual syntax (near-term plan to support)
    ;; binding/declaration syntax
    "mut" "ref" "fun" "mac" "type"
    ;; conditional syntax
    "if" "elif" "else" "and" "or" "not"
    ;; block/loop syntax
    "label"
    ;; module syntax
    "require" "provide" "as"
    ;; control syntax
    "control" "handle" "raise"
    ;; miscellaneous syntax
    "quote" "do" "end")
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
    ("\\_<typeof\\_>" . font-lock-builtin-face)
    ("\\_<error\\_>" . font-lock-warning-face)
    ;; types
    ("\\_<[A-Z][-_a-zA-Z0-9.*\\|]*\\_>" . font-lock-type-face))
  '("\\.rl$")
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
