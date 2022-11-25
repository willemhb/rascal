;;; begin core.rsp
;;; rascal basics module.

;;; globals
(fun walk (fn (xs list))
  (if (nul? xs)
      ()
      (let ((head (fst xs))
      	    (tail (rst xs)))
	   (cons (fn head)
	   	 (map fn tail)))))

(fun walk (fn . xss)
  (if (some? nul? xss)
      ()
      (let ((heads (walk fst xs))
      	    (tails (walk rst xs)))
	   (cons (fn . heads)
	   	 (walk fn tails)))))

(fun keep (p? (xs list))
  (cond ((nul? xs)     ())
  	((p? (fst xs)) (cons (fst xs)
		    	     (keep p? (rst xs))))
	(otherwise     (keep p? (rst xs)))))

(fun range (stop)
  (range 0 stop))

(fun range (start stop)
  (range start stop 1))

(fun range (start stop step)
  (if (>= stop start)
      stop
      (do (yield start)
      	  (range (+ start step)
	  	 stop
		 step))))

(fun for-each (fn generator . args)
  (with (((yield x)
  	  (if (fn x)
	      ()
	      (resume))))
	(generator . args)))

;;; end core.rsp