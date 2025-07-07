# This file is for experimenting with different syntaxes and shouldn't be expected to work.

# module system
#
# Every `rascal` file is evaluated with its own toplevel environment. This means that by default, bindings created in one file aren't visible in others.
# (In general, only bootstrapping C code and repl code alters the global environment). A small API is provided for making these bindings visible.
#
# Module environments are cached globally by absolute path name in `Rascal.__loaded__`.
#
# Module bindings are always final, so an error will be raised if attempts are made to assign to them.

# Binds the module environment to the name `Module` in the global environment.
# Raises an error if this file has previously provided a module name.
# Raises an error if the name `Module` is already bound.
provide Module

# Adds the bindings in `Math` to the current module environment.
# Raises na error if the name `Math` has not been previously provided.
require Math

# Binds the module environment provided by `Math` to the name `M` in the current module environment.
# Raises an error if `Math` is not bound or isn't an environment bound by a provide form.
require Math as M

# Checks `Rascal.__loaded__` to see if a file named "local-file.rl" has been loaded.
# If not, it loads the file and saves the resulting module environment in `Rascal.__loaded__`.
# The module environment associated with "local-file.rl" then has its bindings merged into the
# current module environment.
# "local-file.rl" is first expanded to an absolute path name.
# Errors are raised for the reasons you might imagine (file not found, eval errors, etc).
require "local-file.rl"

# As above, but the module environment associated with "local-file.rl" is bound to the 
require "local-file.rl" as Local

# macro system
mac let(binds, block) do
  
end

mac label(name, binds, block) do
  
end
