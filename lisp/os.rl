(ns os
  "The rascal os module (interface to environment variables, file system, commands, POSIX commands, &c)."

  (fun sys
    "Interface to system calls."
    [cmd])

  (fun env
    "Get or set a system environment variable."
    [x]
    [x y])

  (method sys
    [cmd: String]
    (c-call :system :string [:string] [cmd]))

  (method env
    [x: String]
    (c-call :getenv :string [:string] [x]))

  (method env
    [x: String y: String]
    (c-call :putenv :string [:string] [$"${x}=${y}"]))

  ;; Shortcuts for common environment variables.
  (val &os   (env "GDMSESSION")) ;; not actually valid in general.
  (val &lang (env "LANG"))
  (val &path (env "PATH"))
  (val &home (env "HOME"))
  (val &user (env "USER"))
  (val &host (env "HOSTNAME")))
