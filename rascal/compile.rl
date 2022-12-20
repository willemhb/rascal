;;; begin compile.rl

;;; opcodes
(def op-halt       1)
(def op-nothing    2)
(def op-pop        3)
(def op-dup        4)
(def op-load-nul   5)
(def op-load-true  6)
(def op-load-false 7)
(def op-load-const 8)

(fun op-argco [op]
     (case op
     	   (1 2 3 4 5 6 7)      0 ;; op-halt .. op-load-nul
	   (8 9 10 11 12 13 14) 1
	   (15)                 0
	   (16)                 1
	   (17)                 2
	   (18)                 ))

(fun comp [x]
     ())

(fun comp-file [fname]
     (catch ))

;;; end compile.rl