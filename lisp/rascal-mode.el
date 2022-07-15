;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; (fun a-process
;;;      "example of process syntax/usage."
;;;      ([ :init ] (do-something)
;;;                 (do-something-else))
;;;
;;;      ([ :done ] (do-some-cleanup)
;;;                 (do-some-more-cleanup)))
;;;
;;; (fun (a-process* [initial values])
;;;      "a process with persistent state."
;;;
;;;      (process ([ :init ] (do-something initial)
;;;                          (do-something-else values))
;;;
;;;               ([ :msg1 1 ] (respond initial 1))
;;;
;;;               ([ :msg2 2 ] ())))
;;; 
;;; 
;;; 

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";")
  '( "quote" "do" "do*" "handle"
     "val" "var" "param" ":="
     "def" "fun" "mac" "type" "data" "class" "effect"
     "cond" "case" "and" "or" "if"
     "lambda" "let" "let-mac" )
  '(("else" . font-lock-constant-face)
    ("error" . font-lock-warning-face)
    ("true" . font-lock-builtin-face)
    ("false" . font-lock-builtin-face)
    ("null" . font-lock-builtin-face)
    ("none" . font-lock-builtin-face)
    (":[-+_a-z0-9?!/]+" . font-lock-builtin-face)
    ("@[-_a-z0-9?!/]+" . font-lock-builtin-face)
    ("[-_a-z0-9?!]+([-_ a-zA-Z&]*)" . font-lock-keyword-face))
  '("\\.rsp$" "\\.rdn$" "\\.rdn.o$") ;; source file, serialization format, object file
  '(rainbow-delimiters-mode)
  "A mode for rascal files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here

