;; example of the eventual os library.

(module os
  "POSIX bindings and operating system environment information."
  (import (base math/base))
  (export (environ pwd chdir))
  (export (&home &user &lang &path))

  ;; operating system interface functions.
  @returns String
  (fun environ
    "Get the value associated with `name` in the os environment."
    ((String key))
    (c-call getenv String (String) (key)))

  @returns Symbol
  (fun environ
    "Update the value associated with `name` in the os environment."
    ((String key) (String value))
    (let ((keyval "#{key}={value}")
          (result (c-call putenv Small (String) (keyval))))
      (if (zero? result) :okay :error)))

  @returns String
  (fun pwd
    "Print the current working directory."
    ()
    (c-call pwd String () ()))

  @returns Symbol
  (fun chdir
    "Change the current working directory to that indicated by `path`."
    ((String path))
    (let ((result (c-call chdir Small (String) (path))))
      (if (zero? result) :okay :error)))

  ;; Rascal bindings for important environment variables.
  (val &home (environ "HOME"))
  (val &user (environ "USER"))
  (val &lang (environ "LANG"))
  (val &path (environ "PATH")))