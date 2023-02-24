;;; begin seq.rl

(require "base.rl")
(require "math.rl")

#| generic sequence interface 

   implementations for list, string, 
   binary, vector, tuple, dict, and 
   set types |#

(fn len
  ((list xs))
  (list-len xs))

(fn len
  ((vector xs))
  (vector-len xs))

(fn len
  ((tuple xs))
  (tuple-len xs))

(fn len
  ((string xs))
  (string-len xs))

(fn len
  ((dict xs))
  (dict-count xs))

(fn len
  ((set xs))
  (set-count xs))

(fn empty?
  (xs)
  (zero?))

;;; end seq.rl