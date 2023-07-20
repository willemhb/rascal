;; prelude
(use "base.rl")

;; opcode aliases
(def *op-no-op*          0)
(def *op-start*          1)
(def *op-pop*            2)
(def *op-closure*        3)
(def *op-control*        4)
(def *op-return*         5)
(def *op-load-nil*       6)
(def *op-arg-count*      7)
(def *op-var-arg-count*  8)
(def *op-call*           9)
(def *op-tail-call*     10)
(def *op-load-value*    11)
(def *op-load-global*   12)
(def *op-put-global*    13)
(def *op-def-global*    14)
(def *op-jump*          15)
(def *op-jump-nul*      16)
(def *op-load-closure*  17)
(def *op-put-closure*   18)

(fun literal?
  (expr)
  (if (symbol? expr)
      (not (keyword? expr))
      (not (cons? expr))))

(fun variable?
  (expr)
  (and (symbol? expr)
       (not (keyword? expr))))

(fun macro-call?
  (expr)
  (and (cons? expr)
       (symbol? (hd expr))
       (macro? (lookup (hd expr)))))

(fun special-form?
  (expr)
  (and (cons? expr)
       (has? '(quote do lmb if def put ccc) (hd expr))))