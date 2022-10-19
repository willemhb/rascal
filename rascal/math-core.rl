;;; begin math-core.rsp
;;; export only core language features from math.rsp

(import math (:only number
	     	    + - * /
	     	    zero? one?
	     	    = /= < > <= >=
		    div mod rem))

(export	(number
	 + - * /
	 zero? one?
	 = /= < > <= >=
	 div mod rem))

;;; end math-core.rsp