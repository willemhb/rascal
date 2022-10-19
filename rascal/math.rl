;;; begin math.h

;;; important constants
(val pi
     "A reasonably accurate value for pi (stolen from python3)."
     3.141592653589793)

(val e
     "A reasonably accurate value for e (stolen from python3)."
     2.718281828459045)

(export (pi e))

;;; type definitions
(union number
     (int real))

(export (number))

;;; basic arithmetic operations
;;; +
(fun + "Unary + is just a numeric identity."
     ((x number)) x)
     
(fun + "Binary + calls the appropriate arithmetic primitive."
     ((x int) (y int))
     (%add-int x y))

(fun + "Binary + calls the appropriate arithmetic primitive."
     ((x real) (y real))
     (%add-real x y))

(fun + "Promote to (+ real real)"
     ((x int) (y real))
     (%add-real (real x) y))

(fun + "Promote to (+ real real)"
     ((x real) (y int))
     (%add-real x (real y)))

(fun + "N-ary signature calls itself, reducing the number of inputs by 1."
     (x y . more)
     (+ (+ x y) . more))

;;; -
(fun - "Unary - is sugar for (- 0 x)."
     ((x int)) (%neg-int x))

(fun - "Unary - is sugar for (- 0 x)."
     ((x real)) (%neg-real x))
     
(fun - "Binary - calls the appropriate arithmetic primitive."
     ((x int) (y int))
     (%sub-int x y))

(fun - "Binary - calls the appropriate arithmetic primitive."
     ((x real) (y real))
     (%sub-real x y))

(fun - "Promote to (- real real)"
     ((x int) (y real))
     (%sub-real (real x) y))

(fun - "Promote to (- real real)"
     ((x real) (y int))
     (%sub-real x (real y)))

(fun - "N-ary signature calls itself, reducing the number of inputs by 1."
     (x y . more)
     (- (- x y) . more))

;;; *
(fun * "Binary * calls the appropriate arithmetic primitive."
     ((x int) (y int))
     (%mul-int x y))

(fun * "Binary * calls the appropriate arithmetic primitive."
     ((x real) (y real))
     (%mul-real x y))

(fun * "Promote to (* real real)"
     ((x int) (y real))
     (%mul-real (real x) y))

(fun * "Promote to (* real real)"
     ((x real) (y int))
     (%mul-real x (real y)))

(fun * "N-ary signature calls itself, reducing the number of inputs by 1."
     (x y . more)
     (* (* x y) . more))

;;; /
(fun / "Binary / calls the appropriate arithmetic primitive."
     ((x int) (y int))
     (if (%eq-int y 0)
     	 (raise :division-by-zero)
	 (%div-int x y))

(fun / "Binary / calls the appropriate arithmetic primitive."
     ((x real) (y real))
     (if (%eq-real y 0)
     	 (raise :division-by-zero)
     	 (%div-real x y))

(fun / "Promote to (/ real real)"
     ((x int) (y real))
     (%div-real (real x) y))

(fun / "Promote to (/ real real)"
     ((x real) (y int))
     (%div-real x (real y)))

(fun / "N-ary signature calls itself, reducing the number of inputs by 1."
     (x y . more)
     (/ (/ x y) . more))

(export (+ - * /))

;;; numeric comparison
;;; =
(fun = "Binary signature calls appropriate primitive."
     ((x int) (y int))
     (%eq-int x y))

(fun = "Binary signature calls appropriate primitive."
     ((x real) (y real))
     (%eq-real x y))

(fun = "Promote to (= real real)."
     ((x int) (y real))
     (%eq-real (real x) y))

(fun = "Promote to (= real real)."
     ((x real) (y int))
     (%eq-real x (real y)))

(fun = "N-ary signature calls itself recursively, reducing arity by 1."
     (x y . more)
     (if (= x y)
     	 (= y . more)
	 false))

;;; /=
(fun /= "(compose not =)."
     (x y)
     (not (= x y)))

(fun /= "(compose not =)"
     (x y . more)
     (not (= x y . more)))

;;; <

;;; modulo operations
(fun mod "Alias for (%% x y)"
     ((x int) (y int))
     (%% x y))

(fun mod "C wrapper"
     ((x real) (y real))
     (c-call :fmod
     	     :C-float-64
	     (:C-float-64 :C-float-64)
	     (x y)))

(fun rem "Alias for (%% x y)"
     ((x int) (y int))
     (%% x y))

(fun rem "C wrapper"
     ((x real) (y real))
     (c-call :remainder
     	     :C-float-64
	     (:C-float-64 :C-float-64)
	     (x y)))

;;; math.h bindings
(fun abs "Computes absolute value of an integral value (|x|)."
     ((x int))
     (c-call :labs :C-int-64 (:C-int-64) (x)))


(fun abs "Computes absolute value of an integral value (|x|)."
     ((x real))
     (c-call :fabs :C-float-64 (:C-float-64) (x)))

(fun max "For integers"
     ((x int) (y int))
     (if (> y x) )

(fun exp "")

;;; end math.h