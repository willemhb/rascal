(use (base))

;; Interface to the operating system.
;; Environment variables.
(fun environ (key) @doc "Get the value associated with `key` in the system environment.")
(fun environ (key value) @doc "Set the value assoicated with `key` in the system environment to `value`.")

(fun environ
  ((Symbol key))
  (environ (str key)))

(fun environ
  ((String key))
  (c-call getenv String (String) (key)))

(fun environ
  ((Symbol key) (Symbol value))
  (environ (str key) (str value)))

(fun environ
  ((Symbol key) (String value))
  (environ (str key) value))

(fun environ
  ((String key) (Symbol value))
  (environ key (str value)))

(fun environ
  ((String key) (String value))
  (c-call putenv Small (String) ("#{key}=#{value}")))

;; shortcuts to common environment variables.
(val &path (environ "PATH"))
(val &home (environ "HOME"))
(val &lang (environ "LANG"))

;; directories.
(fun pwd
  @doc "Get the path to the current directory as a string."
  ()
  (c-call pwd String () ()))

(fun cwd
  @doc "Change the current directory to that specified by `path`."
  ((String path))
  (c-call cwd None (String) (path)))

(fun mkdir
  @doc "Create a new directory at the specified `path`."
  ((String path))
  (mkdir path 0))

(fun mkdir
  @doc "Create a new directory at the specified `path` with the given `flags`."
  ((String path) (Small flags))
  (c-call mkdir Small (String Small) (path flags)))

(fun rmdir
  @doc "Remove the directory at the specified `path`."
  ((String path))
  (c-call rmdir Small (String) (path)))

;; miscellaneous.
(fun sys
  @doc "Execute `cmd` in a subshell."
  ((String cmd))
  (c-call system Small (String) (cmd)))
