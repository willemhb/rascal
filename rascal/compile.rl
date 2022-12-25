;;; begin compile.rl

;;; opcodes
(def op-halt          1)
(def op-nothing       2)
(def op-push          3)
(def op-pop           4)
(def op-load-nul      5)
(def op-load-true     6)
(def op-load-false    7)
(def op-load-const    8)
(def op-load-local    9)
(def op-store-local  10)
(def op-load-global  11)
(def op-store-global 12)
(def op-jump-true    13)
(def op-jump-false   14)
(def op-jump         15)
(def op-enter-prompt 16)
(def op-exit-prompt  17)

;;; end compile.rl