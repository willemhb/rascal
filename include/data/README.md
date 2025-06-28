# Overview
This directory contains the definitions of Rascal data types and their supporting functions, types, and globals.

Files implementing individual data types live in the `types/` subdirectory.

Generally, the files in this directory only concern fundamental operations like constructors, destructors, accessors, and methods that dispatch on the type (print, hash, etc).

For objects like `Env` and `Ref` involved in higher-level operations like name resolution, the corresponding APIs live somewhere in the `lang` subdirectory.
