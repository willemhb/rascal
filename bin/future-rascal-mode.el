;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'future-rascal-mode
  '("#" ("\"\"\"" . "\"\"\""))
  '(;; general binding/declaration syntax
    "pub" "mut" "as" "with"
    ;; function binding syntax
    "fun" "mac"
    ;; type binding syntax
    "type" "struct" "union" "trait" "instance" "where"
    ;; conditional syntax
    "if" "else" "cond" "otherwise" "case" "and" "or" "not"
    ;; block/loop syntax
    "label" "loop" "do" "end"
    ;; module syntax
    "module" "import"
    ;; control syntax
    "control" "raise" "from" "when" "handle" "begin"
    ;; process syntaxg
    "receive" "after"
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
    ("\\_<map\\_>" . font-lock-builtin-face)
    ("\\_<filter\\_>" . font-lock-builtin-face)
    ("\\_<reduce\\_>" . font-lock-builtin-face)
    ("\\_<take\\_>" . font-lock-builtin-face)
    ("\\_<drop\\_>" . font-lock-builtin-face)
    ("\\_<zip\\_>" . font-lock-builtin-face)
    ("\\_<send\\_>" . font-lock-builtin-face)
    ("\\_<typeof\\_>" . font-lock-builtin-face)
    ("\\_<error\\_>" . font-lock-warning-face)
    ;; types
    ("\\_<[A-Z][-_a-zA-Z0-9.*\\|]*\\_>" . font-lock-type-face))
  '("\\.rlf$")
  (list 'rainbow-delimiters-mode)
  "A major mode for future rascal files (files with the eventual desired syntax).")

(provide 'future-rascal-mode)
;;; rascal-mode.el ends here
