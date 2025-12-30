# Overview
A Lisp dialect with a focus on extensibility through macros, generic functions, and algebraic effects. Currently in an intermediate state but you can start to get the picture. Check `TODO.md` to get an idea of the current state of the project.

# Features
## Macros
Pretty standard Lisp stuff. Common Lisp style macros with a long term ambition to support some of `Clojure`'s innovations in this field (especially embedding gensyms in syntax templates).

```lisp
(stx when
 (test & body)
 `(if ~test (do ~@body)))
 
(when (isa? 1 Num)
 (println &outs "`when` works as expected."))

(stx var
 (name bind)
 `(def ~name ~bind))

(var x 1)
(put x false) ;; okay

(stx val
 (name bind)
 `(def ~(with-metadata name :final true) ~bind))

(val y 1)
(put y true) ;; error

```

## Generic Functions
All functions are generic, making the language extremely flexible and extinsible. `Julia` pretty much proved that this is the solution to the expression problem and I'm not sure why it isn't the default.

```lisp
(fun conj 
 (x xs: List)
 (cons x xs))

(fun conj
 (g: Glyph cs: Str)
 (str (cons x (chars cs))))

 (fun fmap
  (f: Fun xs: List)
  (if (empty? xs)
   ()
   (cons (f (head xs))
         (fmap f (tail xs)))))

 (fun fmap
  (n: Num xs: List)
  (fmap (fun (x) (ref n x)) xs))
```

Macros are also generic, allowing constructs like docstrings in function definitions to be implemented easily to be implemented easily.

```lisp
(stx fun
 (name: Sym doc: Str formals: List & body)
 `(fun ~(with-metadata name :doc doc) ~formals ~@body))

(fun fmap "This method has a docstring."
 (k: Sym xss: List)
 (fmap (fun (xs) (ref k xs)) xss))
```

## Algebraic Effects
Probably a slight misnomer, but basically an interface for delimited continuations that makes them actually usable.

```lisp
(fun get-input-if-nul 
 (f x)
 (def input
  (if x x (raise :input (fun-name f))))
  (f input))

(handle
 ((op arg k)
  (cond 
   ((=? op :error) (println arg) nul)
   ((=? op :input) (k (getln "need an input for %s: " arg)))
   (otherwise      (raise op arg k))) ;; try next handler
 (get-input-if-nul println)))
```

Obviously that's a pain in the ass but in combination with macros it's an extremely powerful tool for language extension.

```lisp
;; basic exceptions
(stx throw
 (etype message & more)
 `(raise ~etype (list ~message ~@more)))

(stx catch
 (handlers & body)
 (let ((handler-args `(~@(head handlers) _)) ; `k` unused in exceptions.
       (handler-clauses (tail hanlders)))
  `(handle (~handler-args (case ~@handler-clauses)) ~@body)))

;; Python style generators in < 20 lines of code.
(fun yield
 (x)
 (raise :yield x))

(fun make-coro
 (fn)
 (fun (& args)
  (def resume nul)
  (fun inner (x & args)
   (if resume
    (resume x)
    (handle 
     ((op arg k)
      (case op
       (:yield    (put resume k) arg)
       (otherwise (raise op arg k))))
     (apply fn x args))))
  (apply inner args)))

(stx coro
 (args: List & body)
 `(make-coro (fn ~args ~@body)))

(stx coro
 (name: Sym args: List & body)
 `(def ~name (make-coro (fn ~args ~@body))))
```

## Other Stuff

Persistent hash maps and tuples with full reader support.

```lisp
(val example {:x 1, :y 2, :z 3})

(assc example :a 30) ;; {:x 1, :a 30, :y 2, :z 3}

```
