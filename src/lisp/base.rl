;; head/tail compositions
(def hd head)
(def tl tail)

(def ttl
  (lmb (xs) tl.tl.xs))

(def thd
  (lmb (xs) tl.hd.xs))

(def htl
  (lmb (xs) hd.tl.xs))

(def hhd
  (lmb (xs) hd.hd.xs))

(def tttl
  (lmb (xs) tl.tl.tl.xs))

(def tthd
  (lmb (xs) tl.tl.hd.xs))

(def thtl
  (lmb (xs) tl.hd.tl.xs))

(def thhd
  (lmb (x) tl.hd.hd.xs))

(def httl
  (lmb (xs) hd.tl.tl.xs))

(def hthd
  (lmb (xs)
    (head (tail (head xs)))))

(def hhtl
  (lmb (xs)
    (head (head (tail xs)))))

(def hhhd
  (lmb (xs)
    (head (head (head xs)))))

;; binding syntax
(mac fun
  (name formals & body)
  `(def ~name
     (lmb ~formals ~@body)))

(mac labl
  (name args & body)
  (fun args-loop
    (args)
    (if (empty? args)
        ()
        (cons hd.args (args-loop ttl.args)))
  (fun argvals-loop
    (args)
    (if (empty? args)
        ()
        (cons htl.args (args-loop ttl.args))))
  (if list?.name
      `(labl loop ~name ~args ~@body)
      `((lmb ()
          (fun ~name ~args-loop.args ~@body)
          (~name ~@argvals-loop.args))))))

;; basic numeric utilities
(fun zero?
  (n)
  (= n 0))

(fun one?
  (n)
  (= n 1))

(fun even?
  (n)
  (zero? (mod n 2)))

(fun odd?
  (n)
  (one? (mod n 2)))

(fun inc
  (n)
  (+ n 1))

(fun dec
  (n)
  (- n 1))

;; basic sequence utilities
(fun len=?
  (xs n)
  (= (len xs) n))

(fun len<?
  (xs n)
  (< (len xs) n))

(fun len-even?
  (xs)
  (even? (len xs)))

(fun len-odd?
  (xs)
  (odd? (len xs)))

(fun make-list
  (x n)
  (if n.zero?
      ()
      (cons x (make-list n.dec))))

;; more binding syntax
(mac let
  (args & body)
  (def args*
    (labl (argl args accum ())
      (if empty?.argl
          rev.accum
          (loop ttl.argl (cons hd.argl accum)))))
  (def body*
    (labl (argl args accum ())
      (if argl.empty?
          (cat accum.rev body)
          (loop ttl.argl (cons `(put ~hd.argl ~htl.argl) accum)))))
  (def init* (make-list nul len.args*))
  `((lmb ~args* ~@body*) ~@init*))

;; control syntax
(mac cond ()
  (if (len<? &form 3)
      (error &form "not enough expressions in cond")
      (if len-even?.&form
          (error &form "unmatched predicate in cond")
          (labl (test htl.&form, consequent httl.&form, more tttl.form)
            (if empty?.more
                `(if ~test
                     ~consequent
                     (error ~&form "unhandled condition in cond"))
                `(if ~test
                     ~consequent
                     ~@(loop hd.more htl.more ttl.more)))))))

(mac case ()
  (cond (len<? &form 4)
          (error &form "not enough expressions in case")
        (len-odd? &form)
          (error &form "unmatched literal in case")
        :otherwise
          (let (tmp   (gensym)
                xpr   (htl &form)
                cases (ttl &form))
            `(let (~tmp ~xpr)
               ~@(labl (case* (hd cases) consequent (htl cases) more (ttl cases))
                   (if (empty? more)
                       (if (id? case* '_)
                           consequent
                           `(if ~case*
                                ~consequent
                                (error ~&form "unhandled value in case")))
                       `(if (id? ~case* ~tmp)
                            ~consequent
                            ~@(loop (hd more) (htl more) (ttl more)))))))))

(mac and ()
  (cond (len=? &form 1) true
        (len=? &form 2) (htl &form)
        :otherwise      `(if ~(htl &form) (and ~@(ttl &form)))))

(mac or ()
  (cond
    (len=? &form 1) false
    (len=? &form 2) (htl &form)
    :otherwise
      (let (tmp  (gensym)
            xpr  (htl &form)
            rest (ttl &form))
        `(let (~tmp ~xpr)
           (if ~tmp ~tmp (or ~@rest))))))

(mac catch (& body)
  `(ccc throw ~@body))

;; misc syntax
(def *loaded* {})

(mac use (file)
  (let (full-path (expand-file-name file)
        tmp       (gensym))
    `(if (has? *loaded* ~full-path)
         (get *loaded* ~full-path)
         (let (~tmp (load ~full-path))
           (set *loaded* ~full-path ~tmp)))))

;; quasiquote
