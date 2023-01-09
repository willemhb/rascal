;;; begin compile.rl

;;; opcodes
;;; misc.
(def op-begin            0)
(def op-halt             1)
(def op-noop             2)
(def op-pop              3)

;;; load/store
(def op-load-nul         4)
(def op-load-true        5)
(def op-load-false       6)
(def op-load-small-zero  7)
(def op-load-small-one   8)
(def op-load-small-16    9)
(def op-load-const      10)
(def op-load-global     11)
(def op-store-global    12)
(def op-load-local      13)
(def op-store-local     14)

;;; control
(def op-invoke          15)
(def op-return          16)
(def op-jump            17)
(def op-jump-true       18)
(def op-jump-false      19)
(def op-save-prompt     20)
(def op-restore-prompt  21)
(def op-discard-prompt  22)

;;; syntax checking helpers.
(fun check-quote-syntax [form]
     (len=? form 2))

(fun check-do-syntax [form]
     (len>=? form 2))

(fun check-catch-syntax [form]
     (len>=? form 3))

(fun check-if-syntax [form]
     (or (len=? form 3)
     	 (len=? form 4)))

;;; compiler entry point
(fun comp [form]
     (let [prog (module '<toplevel> [] [] #code())]
     	  (catch '()
		 (comp-expr prog form)
	  	 (emit-instr prog op-halt)
	  	 prog)))

;;; toplevel dispatch for compiler.
(fun comp-expr [prog form]
     (cond (lit? form) (comp-lit prog form)
     	   (var? form) (comp-var prog form)
	   otherwise   (comp-combo prog form)))

;;; compile a literal expression.
(fun comp-lit [prog x]
     ;;; helper
     (fun small-16? [x] (between? x -32768 32767))
     ;;; main logic
     (cond (nul? x)      (emit-instr prog op-load-nul)
     	   (true? x)     (emit-instr prog op-load-true)
	   (false? x)    (emit-instr prog op-load-false)
	   (zero? x)     (emit-instr prog op-load-small-zero)
	   (one? x)      (emit-instr prog op-load-small-one)
	   (small-16? x) (emit-instr prog op-load-small-16 x)
	   otherwise     (emit-instr prog op-load-const (put-const prog x))))

;;; compile a variable reference.
(fun comp-var [prog x]
     ;;; helpers
     (fun local-offset [prog x]
     	  (vec-find (module::locals prog) x))
     ;;; main logic
     (let [offset (local-offset prog x)]
     	  (cond offset     (emit-instr prog op-load-local offset)
	  	(bound? x) (emit-instr prog op-load-global (put-const prog x))
		otherwise  (raise x "unbound variable"))))

;;; compile a combination (could be a normal function call a, a special form, or a macro).
(fun comp-combo [prog form]
     (let [head (car form)
     	   tail (cdr form)]
	  (cond (macro-name? head)   (comp-macro-call prog head tail)
	  	(special-form? head) (comp-special prog head tail)
		otherwise            (comp-funcall prog head tail)))

(fun comp-funcall [prog head tail]
     (let [nargs (len tail)]
     	  (comp-expr prog head)
	  (comp-args prog tail)
	  (emit-instr prog op-invoke nargs)))

;;; end compile.rl