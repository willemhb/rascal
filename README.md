# rascal
The first serious version of rascal, a new dialect of lisp inspired by (stealing ideas from) Scheme, Julia, Clojure, Haskell, and Erlang.

rascal is a fully user-extensible language, focusing on support for three key features:
* first-class macros, for adding new syntax
* first-class generic functions, for extending builtin functionality to user-defined types
* first-class algebraic effects, for extending control flow

# key features
## macros
* Clojure-inspired macro system.
  * Common Lisp macros.
  * Practical hygiene is ensured through namespacing and `syntax-quote` form.
  * Syntax quote supports embedded gensyms with `x#` syntax.
  * Namespaces are called modules and have a different surface syntax.

## types & multiple dispatch
* All functions, macros, and effects are generic -- no separate `defmethod`/`defgeneric` forms.
* Practical efficiency is ensured through memoization -- only the first call for a given method signature incurs more-than-constant overhead.

## algebraic effects
* Support for algebraic effects using the `cntl` and `with` forms.
* Algebraic effects allow users to add new control flow to the language -- exceptions, generators, coroutines, actor-based processes, and other control flow can be implemented efficiently in rascal using the builtin effects system.

# other features
* Easy `C` ffi with `c-call` form.
* Users can define new types with the `data` and `union` forms.
* A simple, unobtrusive module/namesapce system requiring only the `import` and `export` forms (with an optional `module` form).
* Scheme-style rest args, as well as an analogous splatting syntax,
  eg `(+ . '(1 2 3 4)) ;; => (apply + '(1 2 3 4))`.
* Builtin support for exceptions using the `catch` form.

# planned features
* Performance improvements (the current version of the compiler doesn't have perform any optimizations).
* Extensible reader (similar to Common Lisp).
* Low latency, concurrent tri-color GC.
* Augmenting the bytecode interpreter with a JIT.
* Pattern matching `case` form.
* Builtin support for `yield` and `async/await` form, implemented using builtin effects system.
* Builtin support for actor-based `process` form, implemented using builtin effects system.
* Standard library modules `math`, `regex`, `io`, `net`, and `sys`.
