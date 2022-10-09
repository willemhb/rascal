# rascal
The first serious version of rascal, a new dialect of lisp inspired by (stealing ideas from) Julia, Scheme, Clojure, Haskell, and Erlang.

rascal is a fully user-extensible language, focusing on support for three key features:
* first-class macros, for adding new syntax.
* first-class generic functions, for extending builtin functionality to user-defined types.
* first-class algebraic effects, for adding new control flow.

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
* Persistent, immutable, HAMT based implementations for the `vector`, `dict`, and `set`.
* Full support for a complementary serialization format based on edn (.rdn, .rdn.o).
* Various performance improvements (the current version of the compiler doesn't perform any optimizations).
* Extensible reader (similar to Common Lisp).
* Low latency, concurrent tri-color GC.
* Augmenting the bytecode interpreter with a JIT compiler producing native code from bytecode.
* Typeclasses using the `class` form.
* Pattern matching `case` form.
* Pattern/template-based macros using `syntax` form.
* Decorator/annotation syntax using familiar `@` syntax.
* Clojure-like function literals, eg `#(+ 1 %) ;; => (fun (%) (+ 1 %))`.
* Clojure-like ref syntax for non-functions, eg `(1 '(1 2)) ;; => (nth 1 '(1 2))` and `(:car '(1 2)) ;; => (ref :car '(1 2))`.
* Implementation of STM semantics, implemented using builtin effects system.
* Builtin support for `yield` and `async/await` form, implemented using builtin effects system.
* Builtin support for actor-based `process` form, implemented using builtin effects system.
* Standard library modules `rdn`, `math`, `regex`, `io`, `net`, and `sys`.
* Standard library modules establishing standard interfaces to be used by 3rd party packages (similar to Python DBAPI 2.0).

# possible features
* `dict` and `set` types might maintain insertion order (this can be done straightforwardly using a method adapted from the CPython `dict` implementation, but I must verify that the space overhead is acceptable).
* Incorporation of some of the syntactic innovations of Bel.
  * dot-application, eg `'(1 2).cdr.car ;; => (car (cdr '(1 2)))`
  * subscript-application, eg `[1 2 3][1] ;; => (1 [1 2 3])`
  * colon-composition, eg `(car:cdr '(1 2)) ;; => ((fun (x) (car (cdr x))) '(1 2))`
* Support for a non-lisp surface syntax.

