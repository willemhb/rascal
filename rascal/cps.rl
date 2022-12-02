;;; begin cps.rl

(fun expression->cps (x k)
 (cond ((literal? x) x)
       ((variable? x) x)
       ((quote-form? x) x)
       (otherwise
        (form->cps x k))))

(fun form->cps (form k)
  (case (car form)
  	((do)      (do->cps form k))
	((if)      (if->cps form k))
	((fun)     (fun->cps form k))
	((def)     (def->cps form k))
	((setv)    (setv->cps form k))
	(otherwise (funcall->cps form k))))

(fun do->cps (xs k)
  (sequence->cps (cdr xs) k))

(fun sequence->cps (xs k)
  (if (len=? xs 1)
      (expression->cps (car xs) k)
      (expression->cps (car xs)
      		       `(fun (_#) ~(sequence->cps (cdr xs) k)))))

(fun quote->cps (xs k)
  `(k ~xs))

(fun if->cps (xs k)
  ())



;;; k, (do <xn>)          => k, <xn>
;;; k, (do <x1> ... <xn>) => (fun (_) <x1>), (do ... <xn>)
;;; 
;;; 

;;; end cps.rl