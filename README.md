# Rascal
The first serious version of rascal, a new functional dynamic language inspired by (stealing ideas from) Julia, Elixir, Scheme, and Clojure.

# Features
* Elixir-inspired syntax.
* Mostly-hygienic syntactic macros.
* Intrinsic multimethods (every function is generic).
* Algebraic effects.
* Others to come.

# Why Elixir?
While Lisp remains the gold standard for syntactic abstraction, most people hate using Lisp, and I wanted to create a language most people would enjoy using. While on the surface
Elixir's syntax isn't remarkable, it comes surprisingly close to Lisp in terms of simplicity and transparency, which has several benefits for macro writers:

* user syntax is superficially indistinguishable from builtin syntax (while this is arguably *not* desirable, it aligns with Rascal's core design goals);
* the relationship between source code and parse tree is relatively easy to visualize;
* the relationship between source code and parse tree is relatively easy to represent in core data structures.

# Why macros?
Syntactic macros (macros operating on syntax trees) are perhaps the second most powerful abstraction tool in the history of programming, after user-defined functions. Though
much-maligned throughout their history, the regular inclusion of syntactic macros in new languages suggests that the good guys have won this point, and I don't have to elaborate
here.

# Why intrinsic multimethods?
Although efficient implementation of multimethods is quite difficult, and although method resolution and ambiguity create occasional headaches for programmers, intrinsic 
multimethods (in the style of Julia) are a vastly simpler and more powerful abstraction tool than the object oriented constructs that have become pervasive in industry and 
languages. In particular:

* easy and intuitive generalization of builtin constructs like `length`, `+`, `<>`, etc. (no need to look up the underlying method);
* no need for additional language concepts like `protocol` or `interface`, nor for Byzantine class hierarchies;
* namespaces remain small and flat, and the best names can be reused;
* dependencies are dramatically reduced or even eliminated, because every function is an interface.

# why algebraic effects?
The central goal of Rascal is to fully realize the promise of Lisp: a language that gives users complete power to implement constructs that don't already exist. While macros 
fulfill much of this promise, they cannot provide (or easily simulate on their own) new non-local control flow constructs. Algebraic effects are not necessarily intuitive or 
easy to use, but they are much more intuitive and usable than formally equivalent constructs like delimited continuations and free monads. My hope is that in combination with 
syntactic macros, they can provide an effective and usable interface for adding new non-local control flow constructs to the language.
