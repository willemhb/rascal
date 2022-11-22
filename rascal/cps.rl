;;; begin cps.rl

(fun expression->cps (x)
 (cond ((literal? x) x)
       ((variable? x) x)
       (otherwise (form->cps x '(fun (v) v)))))

;;; k, (do <xn>)          => k, <xn>
;;; k, (do <x1> ... <xn>) => (fun (_) <x1>), (do ... <xn>)
;;; 
;;; 

;;; end cps.rl