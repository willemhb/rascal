# rascal
The first serious version of rascal, a new dialect of lisp inspired by (stealing ideas from) Julia, Scheme, Clojure, Haskell, and Erlang.

Rascal's features focus on language extension. More broadly, Rascal seeks to build a language and ecosystem where dependency is minimized and programmer freedom is maximized.

# Headline Features
* Almost-hygeinic syntactic macros;
* Dynamic algebraic effects (do not convey type information);
* Julia-style multimethods.

# Other Features
* A simple but proper packaging system;
* `catch`/`throw` syntax (built on top of algebraic effects);
* A small standard library with modules for `core`, `base`, `os`, and `math`.

# Future Features (ordered roughly by difficulty of implementation)
* Support for union and abstract types;
* An extensible reader;
* Extensions to traditional lisp syntax inspired by Clojure and Bel;
* Support for a builtin package manager;
* Support for an `edn`-like serialization format;
* More standard library modules (esp `time`, `text`, `re`, `net`, `sys`, `test`, `rps` (rascal package system), and `rdn` (rascal data notation));
* Builtin concurrency support with two sets of primitives based (respectively) on coroutines and the actor model;
* A single-knob garbage collector that's tunable on a per-actor basis;
* Support for a typeclass, trait, or protocol system;
* Support for predicate dispatch;
* Improvements to the compiler, including support for some kind of JIT (may or may not translate to machine code).

