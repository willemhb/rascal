;; begin stdlib.rl

;; core data types
(def *a-dict*   {:x 1 :y 2})
(def *a-set*   #{1 2 3 4})
(def *a-tuple*  [1 2 3])
(def *a-vec*   #[1 2 3])
(def *a-str*    "abcdefg")
(def *a-bin*   #"000 000 000 001")
(def *a-list*  (1 2 3))

(defn sqr
  [x]
  (* x x))

(defn cub
  [x]
  (* x x x))

(defn dec
  [n]
  (- n 1))

(defn pow
  [x n]
  (cond
    ((zero? n)  1)
    ((even? n)  (pow (sqr x) (/ n 2)))
    (:otherwise (* x (pow x (- n 1))))))

;; end stdlib.rl