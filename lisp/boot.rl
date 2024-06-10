(ns* global

  ;; parsed command line arguments.
  (defvar* &args
    { :doc    => "Command line arguments.",
      :final  => true,
      :public => true,
      :constr => List[String] }
    (command-line-arguments))

  (defvar* &flags
    { :doc    => "Command line flags.",
      :final  => true,
      :public => true,
      :constr => Set[String] }
    (command-line-flags))

  (defvar* &opts
    { :doc    => "Command line options.",
      :final  => true,
      :public => true,
      :constr => Map[String, String] }
    (command-line-options))

  (defvar* &modules
    { :doc       => "Maps paths to loaded modules.",
      :final     => true,
      :public    => true,
      :constr    => MutMap[String, Environ],
      :protected => true }
    (MutMap))

    (defmacro* ))
