# module system
#
# Every `rascal` file is evaluated with its own toplevel environment. This means that by default, bindings created in one file aren't visible in others.
# (In general, only bootstrapping C code and repl code alters the global environment). A small API is provided for making these bindings visible.
#
# Module environments are cached globally by absolute path name in `Rascal.__loaded__`.
# 
# `provide <name>`               - Binds the module environment to the given name in the global environment. Only one `provide` form is allowed per file.
#                                  Attempting to evaluate a second `provide` form raises in an error.
# `require <name>`               - Adds the bindings from an environment previously `provided` into the current environment. An error is raised if no such name has
#                                  been provided.
# `require <name> as <abbrev>`   - Binds the environment previously `provided` in the current environment under `<abbrev>`. An error is raised if `<name>` has not been
#                                  provided. Names in the provided module can be accessed using `<abbrev>.<binding>`.
# `require <path>`               - Checks `Rascal.__loaded__` to see if the `rascal` file at `<path>` has been loaded. If not, the file is executed and its environment
#                                  is cached in `Rascal.__loaded__`. Errors are raised for all the reasons you might imagine (file not found, not a rascal file, eval error,
#                                  etc). Note that if the given file `provides` a name, that name will be available in the global environment.
# `require <path> as <abbrev>`   - Like the above, but makes the given environment available in the current environment under `<abbrev>` rather than merging its bindings
#                                  into the current namespace.

provide Core

# first file loaded in rascal
