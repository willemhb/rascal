;; examples of lisp syntax

;;; type declarations
;;; union type
(type int
      "Basic exaample of a union type."
      (small bigint))

(fun int [x::real]
     (if (> (max small) x)
         (bigint x)
         (small x)))

;;; record type
(type user
      "Basic example of a record type."
      {name age gender communist?})

;;;  struct type (here used to efficiently implement a numeric type).
(type fraction
      "Basic example of a ratio type."
      [numer::int denom::int])

(fun fraction
     "Allow 1 argument."
     [denom::int]
     (fraction 1 denom))

(type ratio
      "Like a fraction, but operations with ratios simplefy aggressively."
      (int fraction))

(fun ratio
     [denom::int]
     (ratio 1 denom))

(fun ratio
     [numer::int denom::int]
     (if (divides? numer denom)
         (/ numer denom)
         (let [cd (gcd numer denom)]
              (fraction (/ numer cd)
                        (/ denom cd)))))

;;; algebraic type (combination of union and struct).
(type tree
     "Basic example of an algebraic type."
     ((empty [])
      (node  [key val (tree left) (tree right)])))

@spec _ -> (>= % 0)
(fun len [::empty] 0)
(fun len [[_ _ l r]::node]
     (+ 1 (len l)
          (len r)))

@spec (apply % _) _ -> _
(fun map [_ ::empty] (empty))

(fun map
     "Inductive case."
     [f [k v l r]::node]
     (node k (f v) (map f l) (map f r)))

@spec (apply % _) _ -> _
(fun filter [_ ::empty] (empty))
