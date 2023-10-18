;; future compiler.

(use (base))

(with
  ((val
    &opcodes
    #table(
      :op-nothing      0,
      :op-pop          1,
      :op-nul          2,
      :op-true         3,
      :op-false        4,
      :op-empty-list   5,
      :op-empty-tuple  6,
      :op-empty-vec    7,
      :op-empty-map    8,
      :op-empty-str    9,
      :op-empty-bits  10,
      :op-zero        11,
      :op-one         12,
      :op-load-i16    13,
      :op-load-g16    14,
      :op-load-val    15,
      :op-load-stack  16,
      :op-put-stack   17,
      :op-load-upval  18,
    ))
   (val
    &argcos
    #table(
      :op-nothing 0,
      :op-pop     0,
      
    )))
  
  (fun compile-expression
    ()))