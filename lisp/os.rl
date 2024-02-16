(module os
  "The rascal os module (interface to environment variables, file system, commands, POSIX commands, &c)."
  (import (core base))
  (export (&os &lang &path &home &user &host
           sys env pwd cwd chown chmod mkdir
           rmdir))
  (begin
    (fun sys
      "Interface to system calls."
      ((String command))
      (c-call :system :int32 (:string) (command)))

    (fun env
      "Lookup os environment variables."
      ((String e-var))
      (c-call :getenv :string (:string) (e-var)))

    (fun env
      "Assign to environment variable."
      ((String e-var) (String e-val))
      (c-call :putenv :int (:string) ((cat e-var "=" e-val))))

    ;; Common environment variables.
    (val &os   (env "GDMSESSION")) ;; not actually valid in general.
    (val &lang (env "LANG"))
    (val &path (env "PATH"))
    (val &home (env "HOME"))
    (val &user (env "USER"))
    (val &host (env "HOSTNAME"))))