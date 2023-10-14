;;; package --- Summary:

;;; keeping this here so it's in the repo

;;; Commentary:
;;; elisp doc formatting is extraordinarily fucked up

;;; Code:
(define-generic-mode
  'rascal-mode
  '(";" ("#|" . "|#"))
  '("fun" "mac" "var" "val"           ;; binding syntax
    "record" "struct" "class" "union" ;; binding syntax (continued)
    "handle" "perform" "resume"       ;; effect syntax
    "guard" "raise" "finally"         ;; exception syntax
    "yield" "async" "sync" "await"    ;; coroutine syntax
    "if" "and" "or"                   ;; branching syntax
    "case" "cond" "not"               ;; branching syntax (continued)
    "let" "with" "label" "loop"       ;; block syntax
    "annot" "impl" "quote" "do"       ;; miscellaneous syntax
    "use" "c-call"                    ;; miscellaneous syntax (continued)
   )
  '(("[^:]:[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ("@[-a-zA-Z?*+0-9/]+" . font-lock-type-face)
    ;; special constants
    ("\\_<nothing\\_>" . font-lock-constant-face)
    ("\\_<nul\\_>" . font-lock-constant-face)
    ("\\_<true\\_>" . font-lock-constant-face)
    ("\\_<false\\_>" . font-lock-constant-face)
    ;; user types
    ("\\_<Float\\_>" . font-lock-type-face)
    ("\\_<Arity\\_>" . font-lock-type-face)
    ("\\_<Small\\_>" . font-lock-type-face)
    ("\\_<Unit\\_>" . font-lock-type-face)
    ("\\_<Boolean\\_>" . font-lock-type-face)
    ("\\_<Glyph\\_>" . font-lock-type-face)
    ("\\_<Symbol\\_>" . font-lock-type-face)
    ("\\_<Function\\_>" . font-lock-type-face)
    ("\\_<Type\\_>" . font-lock-type-face)
    ("\\_<Binding\\_>" . font-lock-type-face)
    ("\\_<Stream\\_>" . font-lock-type-face)
    ("\\_<Big\\_>" . font-lock-type-face)
    ("\\_<Bits\\_>" . font-lock-type-face)
    ("\\_<List\\_>" . font-lock-type-face)
    ("\\_<Vector\\_>" . font-lock-type-face)
    ("\\_<Map\\_>" . font-lock-type-face)
    ;; internal types
    ("\\_<MethodTable\\_>" . font-lock-type-face)
    ("\\_<Native\\_>" . font-lock-type-face)
    ("\\_<Chunk\\_>" . font-lock-type-face)
    ("\\_<Closure\\_>" . font-lock-type-face)
    ("\\_<Control\\_>" . font-lock-type-face)
    ("\\_<Scope\\_>" . font-lock-type-face)
    ("\\_<NameSpace\\_>" . font-lock-type-face)
    ("\\_<Environment\\_>" . font-lock-type-face)
    ("\\_<UpValue\\_>" . font-lock-type-face)
    ;; node types
    ("\\_<VectorNode\\_>" . font-lock-type-face)
    ("\\_<VectorLeaf\\_>" . font-lock-type-face)
    ("\\_<MapNode\\_>" . font-lock-type-face)
    ("\\_<MapLeaf\\_>" . font-lock-type-face)
    ;; fucked up types
    ("\\_<Any\\_>" . font-lock-type-face)
    ("\\_<None\\_>" . font-lock-type-face)
    ("\\_<Term\\_>" . font-lock-type-face)
    ;; numeric type hierarchy (including non-builtin numeric types)
    ("\\_<Integer\\_>" . font-lock-type-face)
    ("\\_<Ratio\\_>" . font-lock-type-face)
    ("\\_<Rational\\_>" . font-lock-type-face)
    ("\\_<Real\\_>" . font-lock-type-face)
    ("\\_<Complex\\_>" . font-lock-type-face)
    ("\\_<Number\\_>" . font-lock-type-face)
    ;; syntax
    ("\\_<otherwise\\_>" . font-lock-function-name-face)
    ("\\_<&\\_>" . font-lock-function-name-face)
    ("\\_<&&\\_>" . font-lock-function-name-face)
    ;; misc
    ("\\_<error\\_>" . font-lock-warning-face))
  '("\\.rl$" "\\.rl.d$" "\\.rl.o$") ;; source file, serialization format, object file
  (list 'rainbow-delimiters-mode)
  "A major mode for rascal2 files.")

(provide 'rascal-mode)
;;; rascal-mode.el ends here
