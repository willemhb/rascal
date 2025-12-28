;; backquote implementation.
;; Helper: check if x is (unquote expr)
(fun unquote? (x)
  (and (list? x)
       (not (empty? x))
       (=? (head x) 'unquote)))

;; Helper: check if x is (unquote-splice expr)
(fun unquote-splice? (x)
  (and (list? x)
       (not (empty? x))
       (=? (head x) 'unquote-splice)))

;; Helper: check if a list contains any unquote-splice at top level
(fun has-splice? (xs)
  (if (empty? xs)
    false
    (or (unquote-splice? (head xs))
        (has-splice? (tail xs)))))

;; Transform a backquoted expression into code that builds the result
(fun bq-transform (x)
  (if
    (not (list? x))
      ;; Atom: quote it
      (list 'quote x)
    (empty? x)
      ;; Empty list: quote it
      (list 'quote x)
    (unquote? x)
      ;; (unquote expr): return the expression
      (head (tail x))
    (unquote-splice? x)
      ;; unquote-splice at top level is an error
      (raise [:syntax-error "unquote-splice (~@) not valid here"])
      ;; Regular list: transform elements
    otherwise
      (bq-transform-list x)))

;; Transform a list, handling splices
(fun bq-transform-list (xs)
  (if (has-splice? xs)
    ;; Has splices: use concat
    (cons 'concat (list (bq-build-segments xs)))
    ;; No splices: use list
    (cons 'list (map bq-transform xs))))

;; Build segments for concat: each segment is either a single-element list
;; or a spliced list
(fun bq-build-segments (xs)
  (if
    (empty? xs)
      ()
    (unquote-splice? (head xs))
      ;; Splice: the expression itself (should be a list)
      (cons (head (tail (head xs)))
            (bq-build-segments (tail xs)))
      ;; Regular element: wrap in list
    otherwise
      (cons (list 'list (bq-transform (head xs)))
            (bq-build-segments (tail xs)))))

;; The backquote macro
(stx backquote
  (expr)
  (bq-transform expr))
