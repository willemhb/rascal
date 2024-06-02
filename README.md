# rascal
The first serious version of rascal, a new dialect of lisp inspired by (stealing ideas from) Julia, Scheme, Clojure, Haskell, and Erlang.

Rascal's features focus on language extension. More broadly, Rascal seeks to build a language and ecosystem where dependency is minimized and programmer freedom is maximized.

# Headline Features
* Almost-hygeinic syntactic macros
* Dynamic algebraic effects (do not convey type information)
* Julia-style multimethods

# Other Features
* A simple module system
* A simple FFI using the `c-call` special form
* Compiles to bytecode for a stack-based virtual machine
* Exceptions and generators built on top of algebraic effects
* A small standard library with modules for `core`, `base`, `io`, `os`, and `math`
* An extensive set of builtin atomic types:
    * `Symbol`, eg `quote`, `toplevel/x`, `toplevel/x#1`, `:keyword`
    * `Number`, eg `12`, `3.14159`
    * `Glyph`, eg `\c`, `\linefeed`, `#\u39B`
    * `Boolean`, eg `true`, `false`
    * `Unit`, eg `nul`
    * `Port`, eg `&ins`, `&outs`, `&errs`, `#port<1451:utf8:read/write>`
* A small but effective set of builtin data structures with mutable and immutable variants (immutable is default, mutable usually prefixed by `#m` in the reader):
    * `List`, eg `(1 2 3)`, `#m(1 2 3)`, `#m(1 2 . 3)`
    * `Binary`, eg `"abc"`, `#m"abc"`, `#utf8"abc"`, `#mutf8"abc"`, `#u8"255 103 005"`, `#mu8"255 103 005"`
    * `Vector`, eg `[1 2 3]`, `#m[1 2 3]`
    * `Map`, eg `{:x 1, :y 2, :z 3}`, `#m{:x 1, :y 2, :z 3}`
* Suppoert for two user-defined data types:
    * user-defined `Struct`, eg `#Node[:key 1, :left nul, :right nul]`
    * user-defined `Record`, eg `#Person{:name "Diane", :age 33, :hot true}`

# Future Features (ordered roughly by difficulty of implementation)
* Support for an `edn`-like serialization format (to be called, creatively, `rdn`)
* Support for reader macros
* Support for a typeclass, trait, or protocol system
* Basic compiler improvements (peephole optimizations, bytecode cache)
* Extensions to traditional lisp syntax inspired by Clojure and Bel
    * Function literals, eg `(map #(+ % 1) [1 2 3 4])`
    * Template function literals, eg ``(map #`(var %1 %2) '((x 1) (y 2) (z 3)))``
    * Function composition inter-symbol operator, eg `(hd|tl '(1 2 3))`
    * Possibly relaxation of parentheses requirements
* Support for union and abstract types
* Support for a builtin package manager (to be titled `rps`)
* More standard library modules (esp `time`, `text`, `re`, `net`, `sys`, `test`, `rps`, and `rdn`)
* Builtin concurrency support with two sets of primitives based (respectively) on coroutines and the actor model
* A single-knob garbage collector that's tunable on a per-actor basis
* Support for some kind of JIT (may or may not translate to machine code)
* Support for predicate dispatch
