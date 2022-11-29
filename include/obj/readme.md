# description
Implementation files for first class Rascal types (for internal types, see `vm/obj`).

For the most part, each builtin type goes in its own file, containing:

* typedef for the C type (if necessary);
* external API functions;
* internal API functions;
* relevant globals (minimmaly the type object, possibly others);
* an initialization function.
* a GC marking function.
