(ns* global
  { :default => true }

  (defvar* &modules
    { :doc       => "Toplevel module cache.",
      :constr    => MutDict }
    #{})

   (defstruct* Module
     { :doc      => "Stores module environment and metadata." }
     [imports: List,
      exports: List,
      body:    List,
      loaded:  Boolean])

   (defvar* &args
     { :doc     => "Command line arguments.",
       :constr  => List,
       :private => false }
     (command-line-arguments))

   (defvar* &flags
     { :doc     => "Command line flags.",
       :constr  => Set,
       :private => false }
     (command-line-flags))

   (defvar* &opts
     { :doc     => "Command line options.",
       :constr  => Map,
       :private => false }
     (command-line-options))

   (assc &modules 'global (environ)))