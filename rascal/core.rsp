;;; begin core.rsp
;;; rascal basics module.

;;; globals
;;; types
(union list
       (cons nul))

(fun list args args)

(export (list))

;;; basic lisp utilities go heare.
(fun map "Basic signature for use inside this module."
     ((fn function) (xs list))
     (if (id? xs nul)
     	 '()
	 (cons (fn (car xs))
	       (map fn (cdr xs)))))

;;; core control flow
(cntl raise "Pass directly to C exit."
      ((x int))
      (c-call :exit :C-void (:C-int-64) (x)))

(cntl raise "Show reason."
      ((x int) msg)
      (do (prin errs msg)
      	  (c-call :exit :C-void (:C-int-64) (x))))

(cntl raise "Ignore."
      (_)
      (c-call :exit :C-void (:C-int-64) (1)))

(cntl raise "Show reason."
      (_ msg)
      (do (prin errs msg)
      	  (c-call :exit :C-void (:C-int-64) (1))))

(cntl finally "Do nothing." (x) x)

(fun try-expand "Helper for use in try."
     (handler)
     if (id? (car handler) 'catch)
     `((cntl raise ~@(cdar handler))
       (do ~@(cdr handler)))
     `((cntl ~@(car handler))
       (do ~@(cdr handler))))

(mac try "Wrapper for raise/finally handlers."
     (handlers . body)
     `(with (((return x) (do (finally x) x))
     	     ~@(map try-expand handlers))
	    (do ~@body)))

(export (try raise finally))

(fun safe-divide "Example of try/catch usage."
     (x y)
     (if (zero? y)
     	 (raise :division-by-zero)
	 (/ x y)))

(fun halve (x)
     (try (prin "Entering halve.\n")
	  (safe-divide 1 0)

	  ((catch x) x)
	  ((catch x msg)
	   (do (prin "#{msg}.\n)") x))
	  ((finally x)
	   (do (prin "Leaving halve.\n")
	       x))

;;; Core control syntax
(mac cond "This signature is only called when exactly
     	   two arguments are left, ensuring correct usage
	   of 'otherwise'."
     (test then)
     (if (id? test 'otherwise)
     	 consequent
	 `(if ~test
	      ~then
	      '(raise :unhandled-case-in-cond))))

(mac cond "This is the common signature."
     (test then . else)
     `(if ~test
     	  ~then
	  (cond ~@else)))

(mac and "base case 1."
     () true)

(mac and "base case 2."
     (x) x)

(mac and "inductive case."
     (x . more)
     `(let (test# ~x)
     	   (if test#
	       (and ~@more)
	       test#)))

(mac or "base case 1."
     () false)

(mac or "base case 2."
     (x) x)

(mac or "inductive case."
     (x . more)
     `(let (test# ~x)
     	   (if test#
	       test#
	       (or ~@more))))

(export (and or cond))
;;; end core.rsp