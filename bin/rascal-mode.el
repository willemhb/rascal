;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#")
  '(;; actual syntax (near-term plan to support)
    ;; binding syntax
    "var" "val" "fun"
    ;; conditional syntax
    "if" "elif" "else" "and" "or" "not"
    ;; block syntax
    "do" "end"
    ;; module syntax
    "require" "provide" "as")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/<>=.]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/<>=.]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/.<>=]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/.<>=]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ("\\_<error\\_>" . font-lock-warning-face)
    ;; types
    ("\\_<[A-Z][-_a-zA-Z0-9.*\\|]*\\_>" . font-lock-type-face))
  '("\\.rl$")
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
