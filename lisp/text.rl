;;; begin text.rl

(require "base.rl")

(type regexp
  { :pattern string, :flags string })

(type regexp-match
  { :regexp regexp,
    :source string,
    :groups })

;;; end text.rl