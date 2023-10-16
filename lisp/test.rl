(use (base enum))

;; basic unit testing framework.

;; example usage:
;;
;; (fun fib
;;   @test (fib 1) 1
;;   @test (fib 2) 1
;;   @test (fib 3) 2
;;   (n)
;;   ())

(mac @test
  @doc "Add a single test to the function's annotations."
  (expression expect)
  (let ((test (thunk (let ((result (eval expression)))
                       (unless (eq? result expect)
                               (raise :assertion-failed "Expression: #{expression}. Expected: #{expect}. Got: #{result}."))))))
    (annot &envt :tests test)))
