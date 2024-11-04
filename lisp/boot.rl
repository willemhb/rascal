(toplevel*
  ;; binding macro definitions.
  (def &required
    "Cache of loaded modules." #{})

  ;; compositions of head/tail and car/cdr
  (fun hhd [x] (hd (hd x)))
  (fun htl [x] (hd (tl x)))
  (fun thd [x] (tl (hd x)))
  (fun ttl [x] (tl (tl x)))

  (fun hhhd [x] (hd (hd (hd x))))
  (fun hhtl [x] (hd (hd (tl x))))

  (fun caar [x] (car (car x)))
  (fun cadr [x] (car (cdr x)))
  (fun cdar [x] (cdr (car x)))
  (fun cddr [x] (cdr (cdr x))))