;;; package --- Summary:

;;; keeping this here so it's in the repo.

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up.

;;; Code:
(define-generic-mode
  'rascal-lisp-mode
  '(";" ("#|" . "|#"))
  '( ;; special forms
    ;; binding forms
    "def" "put" "fn"

    ;; control forms
    "if" "quote" "do"

    ;; continuations
    "call-with-handler"

    ;; miscellaneous forms
    "annot"

    ;; extended syntax
    ;; binding syntax
    "var" "val" "fun" "mac" "method" "λ" "type" "struct" "record"

    ;; block/looping syntax
    "let" "label" "loop"
    
    ;; conditional syntax
    "cond" "case" "and" "or" "when" "unless"

    ;; control syntax
    "handle" "raise" "raise*" "begin"
    
    ;; module macros
    "require" "provide")
  '(("\\(?:[^-_a-zA-Z?*+0-9/:]:[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^:[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ("\\(?:[^~]@[-_a-zA-Z?*+0-9/]+\\)\\|\\(?:^@[-_a-zA-Z?*+0-9/]+\\)" . font-lock-type-face)
    ;; special constants
    ("\\_<none\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
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
