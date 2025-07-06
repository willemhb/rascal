;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-mode
  '("#" ("#|" . "|#"))
  '( ;; syntax
    ;; basic binding syntax
    "def" "var" "val"
    ;; function binding syntax
    "fun" "mac"
    ;; type binding syntax
    "union" "struct"
    ;; conditional syntax
    "if" "else" "elif" "and" "or" "not" "case"
    ;; looping syntax
    "label" "loop"
    ;; effect handling syntax
    "control" "handle" "begin" "raise" "from"
    ;; module syntax
    "module" "import" "as"
    ;; block syntax
    "let" "do" "end"
    ;; miscellaneous syntax
    "quote")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/<>=.]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/<>=.]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/.<>=]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; types
    ("\\_<[A-Z][-_a-zA-Z0-9.*\\|]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("::" . font-lock-comment-face))
  '("\\.rscl$")
  (list 'rainbow-delimiters-mode
                                        ; (lambda () (set-input-method 'TeX))
        )
  "A major mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
