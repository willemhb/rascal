#| functions and macros for bootstrapping the language.
   mostly very simple core utilities and helpers for syntactic analysis. |#

(def
  ^{:final true
    :type  Function
    :doc   "Type predicate for Lists."}
   list?
  ^{:signature [Any]}
   (fn (x) (isa? x List)))

(def
  ^{:fianl true
    :type  Function
    :doc   "Type predicate for Symbols."}
    symbol?
    ^{:signature [Any]}
    (fn (x) (isa? x Symbol)))

(def
  ^{:final true
    :type  Function
    :doc   "Type predicate for Types."}
  type?
  ^{:signature [Any]}
  (fn (x) (isa? x Type)))

(def
  ^{:final true
    :type  Function
    :doc   "Pseudo-type predicate for non-empty Lists."}
  cons?
  )

(def
  ^{:macro true
    :final true
    :type  Function
    :doc   "Standard syntax for creating new macro bindings."
    :})
