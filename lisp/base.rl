;; Definitions of basic standard library functions and features. Most Rascal files begin with

(ns base
  "Rascal standard library base.")

(use (core))

(mac hndl
  "Wrapper around primitive `hndl*`."
  [(List handler) & body]
  `(hndl*
    (fn* ~@handler)
    (fn* [] ~@body)))

(mac raise
  "Wrapper around primitive `raise*`."
  [(Sym op) & args]
  `(raise* ~op [~@args]))

(mac raise
  "")

(mac catch
  "Simplefied interface for")
