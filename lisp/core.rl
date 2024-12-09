;; The second Rascal file loaded by the interpreter on startup.
;; Primarily responsible for defining core Rascal syntax (fun, mac, let, require, cond, and, or, etc).

(ns* core)

(def* mac
  { :macro => true,
    :doc   => "Standard syntax for creating new macro bindings." }
  (Func 'mac))

(add-method! mac
  (fn* [(List &form) (Env &env) (Sym name) (Vec args) & body]
    (if* (defined? name &env)
      `(add-method! ~name (fn* [(List &form) (Env &env) ~@args] ~@body))
      `(do ))))

