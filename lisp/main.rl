(load "prelude.rl")

(mac val
  (name bind)
  `(do (var ~name ~bind)
       (freeze! ~name)))

 (mac and
   (& body)
   (if (len=? body 0) true
       (len=? body 1) (hd body)
       :otherwise     `(let (test# ~(hd body))
                         (if test#
                             (and ~@(tl body))
                             test#))))

(repl)
