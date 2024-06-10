(module io
  "Builtin io library."
  (import [base])
  (export [&in, &out, &err,
           read, write, prn])
  (begin

    (val &in
      "Standard input stream."
      (*standard-input*))

    (val &out
      "Standard output stream."
      (*standard-output*))

    (val &err
      "Standard error stream."
      (*standard-output*))))