;;; begin os.rsp
;;; interface to operating system

;;; mapping from symbols to exit statuses
(val *exit-statuses*
     {
	:okay  => 0,
	:error => 1,
     })

(fun sys "Convert to (sys string)."
     ((cmd symbol))
     (sys (string cmd)))

(fun sys "Binding for C builtin function 'sys'."
     ((cmd string))
     (c-call :system :C-int-32 (:C-pointer) (cmd)))

(fun exit "Lookup status in *exit-statuses*."
     ((status symbol))
     (do (prin "exiting. status: #{status}.\n")
	 (exit (ref status *exit-statuses* :fallback 1))))

(fun exit "Binding for C builtin function 'exit'."
     ((status int))
     (c-call :exit :C-void (:C-int-32) (status)))

(fun getenv "Convert to (getenv string)."
     ((var symbol))
     (getenv (string var)))

(fun getenv "Binding for C builtin function 'getenv'."
     ((var string))
     (c-call :getenv :C-pointer (:C-pointer) (var)))

(fun chdir "Binding for C builtin function 'chdir'."
     ((path string))
     (c-call :chdir :C-int-32 (:C-string) (path)))

(fun getcwd "Binding for C builtin function 'getcwd'."
     ()
     (c-call :getcwd :C-string () ()))

(export (sys getenv exit chdir getcwd))

;;; end os.rsp