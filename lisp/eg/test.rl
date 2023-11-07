;; example unit testing framework.

(module test
  "Simple decorator-based unit testing framework."
  (import (base))
  (export (assert equals raises test))

  (fun add-test
    "Update the tests associated with the given object. Create the `:tests` object if necessary."
    (object test-type expression sentinel)
    (unless (has? (annot object) :tests)
      (annot object :tests {:equals (), :raises ()}))
    (let ((tests  (annot object :tests))
          (anns   (ref tests test-type))
          (anns*  (conj anns [expression sentinel]))
          (tests* (xef tests test-type anns*)))
      (annot object :tests tests*)))

  (mac assert
    "Simple assert macro. A no-op if `&debug` is `false`."
    (assertion)
    (when &debug
      `(when (not ~assertion)
         (raise :assertion-failed '~assertion))))

  (mac equals
    "Adds an equality test."
    (object expression result)
    (add-test object :equals expression result)
    object)

  (mac raises
    "Adds a test to check that the given expression raises the supplied exception."
    (object expression exception-type)
    (add-test object :raises expression exception-type)
    object)

  (fun test
    "Run the tests supplied for a given function."
    ((Function fn))
    (when (has? (annot fn) :tests)
      (let (({equals :equals, raises :raises} (annot fn :tests)))
        ;; run equality tests.
        (for [x, v] in equals do
          (let ((v0 (eval x)))
            (unless (eql? v0 v)
              (raise :test-failed "Expected `#{x}` to evaluate to `#{v}`, got `#{v0}` instead."))))
        ;; run raises tests.
        (for [x, s] in raises do
          (guard
            (((type & _)
              (unless (eql? xtype s)
                (raise :test-failed "Expected `#{x}` to raise `#{s}`, got `#{xtype}` instead."))))
            (let ((x0 (eval x)))
              (raise :test-failed "Expected `#{x}` to raise `#{s}`, returned `#{x0}` instead."))))))))