;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-lisp-mode
  '(";" ("#|" . "|#"))
  '( ;; binding syntax
    ;; general binding syntax
    "def" "var" "val" "put"
    ;; function binding syntax
    "fn" "fun" "mac"
    ;; type binding syntax
    "struct" "union"
    ;; control syntax
    ;; branching syntax
    "if" "and" "or" "cond" "not"
    ;; miscellaneous control syntax
    "quote" "do"
    ;; effect syntax
    "control" "handle" "begin" "raise" "from"
    ;; block/looping syntax
    "let" "label" "loop"
    ;; module syntax
    "module" "import"
    ;; metadata syntax
    "annot")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/.]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ("\\_<not\\_>" . font-lock-builtin-face)
    ;; types
    ("\\_<[A-Z][a-zA-Z0-9\\|]*\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face))
  '("\\.rl$")
  (list 'rainbow-delimiters-mode
                                        ;  (lambda () (set-input-method 'TeX))
        )
  "A major mode for rascal files.")

(provide 'rascal-lisp-mode)
;;; rascal-mode.el ends here
