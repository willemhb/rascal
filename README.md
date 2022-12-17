# rascal
The first serious version of rascal, a new dialect of lisp inspired by (stealing ideas from) Julia, Scheme, Clojure, Haskell, and Erlang.

## key features
   * `clojure` style macros.
   * `julia` style multiple dispatch - all functions are generic (no separate `defgeneric`/`defmethod`).
   * algebraic effects for extensible control flow.
   * simple module system.
   * user-extensible type system with support for unions, records, and type aliases.
   
## in-depth tour
### types
#### types are sets
Types denote a (possibly infinite) set of values. The upshot of this is that all types have a uniform representation. 

#### types are represented by constructors
Every type has a constructor, a pure function that returns values from the set associated to the type, or signals an error if no such value can be found. The constructor
is the value used to represent the type within rascal. Eg, the function `typeof` returns a function object, not a type object.

The default behavior of a constructor `C` for type `T` with argument `x` is to return `x` if `x` is a member of `T`, and to signal an error otherwise.

#### primitive types

#### union types

#### record types

### multiple dispatch

### macros

### effect system

### module system
#### importing and exporting
