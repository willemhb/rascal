#ifndef rascal_builtin_h
#define rascal_builtin_h

#include "rascal.h"
#include "describe/function.h"

// builtin functions and validators -------------------------------------------

// constructors ---------------------------------------------------------------
DeclareBuiltin(boolean);
DeclareBuiltin(character);
DeclareBuiltin(integer);
DeclareBuiltin(fixnum);
DeclareBuiltin(symbol);
DeclareBuiltin(pair);
DeclareBuiltin(cons);
DeclareBuiltin(table);
DeclareBuiltin(string);
DeclareBuiltin(binary);
DeclareBuiltin(tuple);
DeclareBuiltin(error);

// general predicates ---------------------------------------------------------
DeclareBuiltin(idp);   // id?
DeclareBuiltin(eqlp);  // =?
DeclareBuiltin(isap);  // isa?

// basic utilities ------------------------------------------------------------
DeclareBuiltin(ord);
DeclareBuiltin(sizeof);
DeclareBuiltin(typeof);

// high level assignment ------------------------------------------------------
DeclareBuiltin(setv);

// arithmetic -----------------------------------------------------------------
DeclareBuiltin(add);
DeclareBuiltin(sub);
DeclareBuiltin(mul);
DeclareBuiltin(div);
DeclareBuiltin(mod);
DeclareBuiltin(eqp);
DeclareBuiltin(ltp);

// accessors ------------------------------------------------------------------
DeclareBuiltin(car);
DeclareBuiltin(cdr);
DeclareBuiltin(xar);
DeclareBuiltin(xdr);
DeclareBuiltin(ref);
DeclareBuiltin(xef);
DeclareBuiltin(put);
DeclareBuiltin(xut);


// sequences & collections ----------------------------------------------------
DeclareBuiltin(len);          // len
DeclareBuiltin(emptyp);       // empty?
DeclareBuiltin(hasp);         // has?
DeclareBuiltin(fst);          // fst
DeclareBuiltin(rst);          // rst
DeclareBuiltin(join);         // join - add at front
DeclareBuiltin(append);       // append - add at back
DeclareBuiltin(conj);         // conj - add in optimal manner

// low-level io ---------------------------------------------------------------
DeclareBuiltin(open);                    // open
DeclareBuiltin(close);                   // close
DeclareBuiltin(princ);                   // princ
DeclareBuiltin(readc);                   // readc
DeclareBuiltin(peekc);                   // peekc

// character and string handling ----------------------------------------------
DeclareBuiltin(ctypep);                  // char-type?
DeclareBuiltin(upper);                   // upper
DeclareBuiltin(lower);                   // lower

// core -----------------------------------------------------------------------
DeclareBuiltin(read);                    // read
DeclareBuiltin(eval);                    // eval
DeclareBuiltin(prin);                    // prin
DeclareBuiltin(load);                    // load
DeclareBuiltin(comp);                    // comp
DeclareBuiltin(exec);                    // exec

// low-level environment interaction ------------------------------------------
DeclareBuiltin(lookup);                  // lookup
DeclareBuiltin(boundp);                  // bound?
DeclareBuiltin(extendb);                 // extend!
DeclareBuiltin(bindb);                   // bind!
DeclareBuiltin(assignb);                 // assign!
DeclareBuiltin(captureb);                // capture!

// system interaction ---------------------------------------------------------
DeclareBuiltin(exit);                    // exit

// special forms --------------------------------------------------------------
DeclareBuiltin(quote);
DeclareBuiltin(lambda);
DeclareBuiltin(macro);
DeclareBuiltin(do);

value_t rsp_if(value_t args, size_t nargs);
bool_t  ensure_if(value_t args, size_t nargs);

#endif
