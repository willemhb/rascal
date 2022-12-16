;;; begin core.rl
;;; rascal basics module.

;;; globals
;;; example of defining a custom type and implementing methods
(enum Tree
      [(Empty) :as EmptyTree
       (Leaf key val)
       (Node (left Tree) (right Tree))])

(export [Tree length map filter])

(fun length "Implement for Tree()."
  [(tree Tree)]
  (case tree
    [EmptyTree  0]
    [(Leaf _ _) 1]
    [(Node l r)
     (+ (length l) (length r))]))

(fun map "Implement for Tree()."
  [fn (tree Tree)]
  (case tree
    [EmptyTree  EmptyTree]
    [(Leaf k v) (Leaf k (fn v))]
    [(Node l r) (Node (map fn l)
    	     	      (map fn r))]))

(fun filter "Implement for Tree()."
  [p? (tree Tree)]
  (case tree
    [EmptyTree  EmptyTree]
    [(Leaf k v) (if (p? k) tree EmptyTree)]
    [(Node l r) (Node (filter p? l)
    	     	      (filter p? r))]))



;;; end core.rl