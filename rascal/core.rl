;;; begin core.rsp
;;; rascal basics module.

;;; globals
(fun map (fn (xs list))
  (if (nul? xs)
      ()
      (let ((head (fst xs))
      	    (tail (rst xs)))
	   (cons (fn head)
	   	 (map fn tail)))))

(fun map (fn . xss)
  (if (some? nul? xss)
      ()
      (let ((heads (map fst xs))
      	    (tails (map rst xs)))
	   (cons (fn . heads)
	   	 (map fn tails)))))

;;; end core.rsp