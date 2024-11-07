# Phase 1
This phase should implement a minimal but still interesting language on which the more complete version of Rascal can be built.

## Highlights
* A stack-based virtual machine with upvalue-based closures.
  * Using a two stack design, one for values and one for call state.
* VM support for exceptions (accessed in Rascal with `catch*` and `throw*` special forms).
* VM support for a basic module system (no module nesting, idempotent `require` form, can rename bindings from module using `(require "module.rl" :as module)`).
* Support for the following user types:
  * Nul
  * Bool
  * Glyph
  * Num
  * Port
  * Str
  * UserFn
  * Sym
  * Pair     (mutable)
  * Buffer   (mutable)
  * Alist    (mutable)
  * Table    (mutable)

* Support for the following VM types:
  * Ptr
  * NativeFn
  * Environ
  * Ref

* `math`, `os`, `txt`, and `io` standard library modules
* Basic C ffi with `c-call` interface
* Support for metadata
* Support for macros
* Reader support for backquote with symbol interpolation (`#x`, `#y`)
* Reader support for nul, booleans, glyphs, numbers, symbols, strings, conses, binaries, vectors, and tables

# Phase 1.5
This phase will focus on fleshing out and improving the basic infrastructure put in place in Phase 1 before moving on to the more challenging features.

## Highlights
* Full support for persistent counterparts to basic types introduced in Phase 1:
  * List   (persistent Pair)
  * Binary (persistent Buffer)
  * Vector (persistent Alist)
  * Map    (persistent Table)

* Basic support for an expanded type system:
  * Support for Any, None, Builtin, Struct, Record, and Union types
  * Types become first-class values
  * Users able to create new Struct, Record, and Union types
  
* Expanded namespace/module system
  * Add support for nested modules
  * Add support for limited imports
  * Introduce notion of qualified symbols

