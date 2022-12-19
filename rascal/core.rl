;;; begin core.rl
;;; rascal basics module.

;;; core effects
@doc "Basic let implementation."
(mac let
     [(vec .. keys vals) .. body]
     `((fun [~@keys] ~@body) ~@vals))

@doc "If with no consequent and implicit do."
(mac when
     [test .. body]
     `(if ~test
          (do ~@body)))

@doc "Complement of when."
(mac unless
     [test .. body]
     `(if (not ~test)
          (do ~@body)))

@doc "Basic catch implementation."
(mac catch
     [(vec .. (list (vec args) .. handler)) .. body]
     (let [args (map )]))

;;; end core.rl