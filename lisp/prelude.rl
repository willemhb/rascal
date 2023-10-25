;; All of the standard function definitions to be used in core.rl

@final
@type Function
@doc "Alias for `head`."
(def hd head)

@final
@type Function
@doc "Alias for `tail`."
(def
  @final
  @type Function
  @doc "Alias for `tail`."
  tl
  tail)

(def
  @final
  @type Function
  hhd
  (lmb
    ((List xs))
    (hd (hd xs))))

;; macros that are either very simple or useful for making other macros.
