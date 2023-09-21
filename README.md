# Rascal
The first serious version of rascal, a new functional dynamic language inspired by (stealing ideas from) Julia, Elixir, Scheme, and Clojure.

# Features
* Elixir-inspired syntax.
* Mostly-hygienic macros.
* Intrinsic multimethods (every function is generic).
* Algebraic effects.
* Others to come.

# Why Elixir?
While Lisp remains the gold standard for syntactic abstraction, Elixir's syntax ends up coming quite close. But people hate using Lisp, and I wanted to create a language most people would
enjoy using.

At its core Elixir's syntax is just the mathematical notation taught in primary and secondary education plus a very small number of transformations that nevertheless give it the full expressive 
power of other non-Lisps. This has several beneficial implications for macro writing:

* user syntax is superficially indistinguishable from builtin syntax;
* the relationship between source code and parse tree is relatively easy to visualize;
* the relationship between source code and parse tree is relatively easy to represent in core data structures.

# Why macros?

# Why intrinsic multimethods?

# why algebraic effects?
