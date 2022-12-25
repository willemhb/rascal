# Early development
The focus of versions 0.0.0 - 0.0.16 focus on incrementally implementing a servicable base 
language, culminating in the bootstrapping of a simple `rascal` compiler.

## Version 0.0.0 - echo interpreter
### Goals
This "version" is just getting the repl and the object model in place.

### Target features
* `real` type implemented
* `read`, `eval`, `prin`, `repl` interpreter primitives implemented

## Version 0.0.1 - names
### Goals
Add support for symbols (they don't do anything yet).

### Target features
* `sym` type implemented (with read/prin support).

## Version 0.0.2 - lists
### Goals
Add support for lists.

### Target features
* `cons`, `nul` types implemented (with read/prin support).

## Version 0.0.3 - arrays
### Goals
Add support for array types used by the compiler.

### Target features
* `vec`, `code` types implemented (with read/prin support).

## Version 0.0.4 - s-expressions & symbol values (current)
### Goals
To make lists meaningful, implement function calls (and functions).

Implement toplevel symbol values at the same time.

### Target features
* `module`, `prim`, `native` types implemented
* `sym` type updated
* `eval` primitive updated
* `+`, `-`, `*`, `/`, `=`, `<` arithmetic primitives implemented
* `cons`, `car`, `cdr` list primitives implemented
* `vec`, `vec-len`, `vec-ref`, `vec-xef`, `vec-push`, `vec-pop` vec primitives implemented
* `comp`, `exec` interpreter primitives implemented
* `dis` debugging primitive implemented

## Version 0.0.5 - special forms
### Goals
Introduce special forms and add support for basic forms.

### Target features
* `quote`, `do` forms implemented.

## Version 0.0.6 - catch
### Goals
Add basic error handling early on.

### Target features
* `small` type implemented.
* `catch` form implemented.
* type and arity checking added to function calls.
* `error` runtime primitive added.

## Version 0.0.7 - assignment
### Goals
Add toplevel assignments to the interpreter.

### Target features
* `def`, `put` forms implemented.
* `lookup` symbol primitive implemented.

## Version 0.0.8 - branching
### Goals
Add basic branching to the interpreter.

### Target features
* `if` form implemented.
* `bool` type implemented.
* `id?`, `not`, `isa?`, `bound?` predicate primitives added.

## Version 0.0.9 - abstractions
### Goals
Add support for user-defined functions.

### Target features
* `fun` form implemented.
* `[x y .. rest]` variadic argument support implemented.
* `module` type updated.

## Version 0.0.10 - closures
### Goals
Add support for nested functions (this is a big one).

### Target features
* Add support for nested functions, closures, and upvalues.
* `closure`, `upval` types implemented.

## Version 0.0.11 - basic io
### Goals
In preparation for bootstrapping the compiler, begin introducing IO types.

### Target features
* `ios` type implemented.
* `open`, `close`, `eos?` stream primitives implemented.
* `ios` type integrated into `read`/`prin`.

## Version 0.0.12 - text
### Goals
Add basic text types.

### Target features
* `str`, `glyph` types implemented.
* `glyph-type`, `glyph-type=?`, `upper`, `lower` character primitives added.
* `str-len`, `str-ref`, `str-cat`, `str-rev`, `str-upper`, `str-lower`, string primitives added.
* `princ`, `readc`, `peekc` stream primitives added.

## Version 0.0.13 - running files
### Goals
Add support for executing source files.

### Target features
* `load` interpreter primitive added.

## Version 0.0.14 - object files
### Goals
Add support object files (saved compiled bytecode).

### Target features
* object file format implemented.
* `comp-file` compiler primitive added.

## Version 0.0.15 - preparing to bootstrap
## Goals
Implement a few additional branching forms to ease writing the compiler. Also add in basic
garbage collection at this point.

## Target features
* `and`, `or`, `cond`, `case` forms implemented.
* automatic memory management implemented.

## Version 0.0.16 - bootstrapping
### Goals
At this point we seek to begin using rascal to implement rascal.

### Target features
* rewrite `comp` in rascal.
* compile `comp`.

# Middle development
The focus of versions 0.0.17 - 0.0.28 is to add the more sophisticated features that make
rascal distinctive. This phase is focused on basic implementations for these features, not
efficient implementations. Refinement will come later.

## Version 0.0.17 - hashing & cacheing
### Goals
This version is focused on the implementation of sophisticated hashing and comparison that 
works for all values and handles reference cycles in hashed data. These features will be 
needed for the development of more advanced features.

### Target features
* `dict`, `set` types implemented
* `hash`, `eq?`, `ord` comparison and hashing primitives implemented

# Version 0.0.18 - packages
## Goals
Version 0.0.17 focuses on implementing the core of an extremely simple package system.

Packages are implemented on top of closures. Packages are the result of executing a file.

Packages are loaded at most once, and stored in the toplevel namespace.

## Target features
* `require`, `provide` forms implemented
* `ns`, `var`, `pkg` types implemented
* `sym`, `module` types updated

# Version 0.0.19 - macros
## Goals
With the package system in place, Version 0.0.19 focuses on implementing the basic macro system.

## Target features
* `mac` form implemented
* `macro-call?`, `macro-expand-1`, `macro-expand` macro primitives implemented
* `ns`, `var`, `pkg` types updated

# Version 0.0.20 - backquote
## Goals
Once macros and packages exist, it's time to improve the reader. Version 0.0.20 is focused on
extending the reader to support a `clojure`-like backquote form. Custom reader macros are
left for a later date.

A few other syntaxes are implemented for convenience.

## Target features
* `~backquote`, `~@backquote`, `backquote#` syntaxes implemented
* `%(+ 1 %)` lambda literal syntax implemented

# Version 0.0.21 - arity overloading, variadic functions, and splatting
## Goals
As a first step toward true multi-methods, this version introduces overloading by function
arity and rest arguments. It also updates the `fun` and `mac` syntax to support multiple
signatures.

## Target features
* `func` type implemented
* `(+ 1 2 .. '(3 4))` splatting syntax implemented
* overloading by function arity implemented (prelude to multimethods)

# Version 0.0.22 - basic type annotations
## Goals
In order to allow dispatching on types, we need a syntax for annotating types. This update 
focuses on meeting that requirement.

## Target features
* `(<type> <var>)` type annotation syntax implemented
* annotation support implemented in `fun`, `mac`, `let`, and `def` forms
* runtime type checking implemented

# Version 0.0.23 - true multimethods
## Goals
With basic dispatch and variable types implemented, full multiple dispatch is added.

Fast dispatch is not the focus of this version.

## Target features
* `method-table`, `method` types implemented
* multiple dispatch implemented in the interpreter

# Version 0.0.24 - first class types
## Goals
Support for user types requires types to be first class objects. Basic support for this is the 
focus of this update.

## Target features
* `type` form implemented
* `data-type`, `alias-type` types implemented, integrated into object system

# Version 0.0.25 - record types
## Goals
With first class types in place, it's time to implement non-trivial user types. We start with
record types.

## Target features
* `type` form extended with support for `(type <name> {..<fields>})` syntax.
* `record`, `record-type` types implemented, integrated into object system.

# Version 0.0.26 - union types
Extend the type system to support unions.

# Version 0.0.27 - continuations

# Version 0.0.28 - first-class effects
## Goals


# Late development
At this point rascal is nearly ready to be public. Focus in this stage is on improving performance,
working on a standard library, and implementing features that are nice for usability but not essential
to development.

This would be the point in development when I might consider bringing in other people.

## Version 0.0.29 - functional vec
### Goals
Reimplement the `vec` type using persistent HAMTs.

### Target features
* `vec-node` type added.
* `vec` type updated.

## Version 0.0.30 - functional set
### Goals
Same as above for `set`

### Target features
* `set-node`, `set-leaf`, `set-leaves` types added.
* `set` type updated.

## Version 0.0.31 - functional dict
### Goals
Same as above for `dict`

## Target features
* `dict-node`, `dict-leaf`, `dict-leaves` types added.
* `dict` type updated.

## Version 0.0.31 - sequences & collections
### Goals
With multimethods implemented, we can implement a generic interface for sequences.

## Target features
* `list` type implemented.
* `first`, `rest`, `nth`, `empty?`, `elem?`, `any?`, `all?`, 
  `for-each`, `map`, `filter`, `reduce`, `take`, `drop`, `conj`
  interface functions implemented for `list`, `vec`, `set`, `dict`, and `str`
  types.

## Version 0.0.32 - faster dispatch
### Goals
Improve the speed of method dispatch.

### Target features
* Add cacheing to method dispatch.

## Version 0.0.33 - destructuring
### Goals
Object destructuring helps cut down on the number of parentheses in many cases, so add
support for it at this point.

### Target features
* Add support for `(cons head _)` destructuring syntax.
* Add support for `(vec first .. last)` destructuring syntax.
* Implement destructuring in `fun` and `let` forms.

## Version 0.0.34 - struct types
### Goals
There are obvious benefits, especially in a dynamic language, of types whose keys can
be added dynamically (as in `JavaScript`, `Python`, etc). However, these are terrible
for implementing data structures and algorithms. Rascal seeks to solve this by separating
user composite types into `struct`s and `record`s. 

`record`s, implemented in 0.0.25, will be implemented on top of `dict`s, and allow adding 
and removing keys at runtime. `struct`s, implemented in this update, are fixed, use
space much more efficiently, and (potentially) support more advanced type features.

## Target features
    * Add `struct` and `struct-type` support.
    * Add support for `(type [.. <fields>])` syntax.
