;; (control
;;   (fn () (/ 2 0))
;;   (fn (err)
;;     (print &outs err)
;;     (newline &outs) err))

;; simple backquote test
;; (stx when
;;  (test & body)
;;  `(if ~test
;;    (do ~@body)))

;; (when true
;;  (print "when works as expected.")
;;  (newline))
