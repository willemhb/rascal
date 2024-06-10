(module core
  "The rascal core module (mostly bootstrapping and macro definitions)."
  (export [val, var, put,
           fun, mac,
           type, abstract, union, record, struct, trait,
           let, label, loop, for, while,
           cond, case, and, or,
           throw, catch,
           yield, gen]))
