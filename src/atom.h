#ifndef rascal_atom_h
#define rascal_atom_h

#include "table.h"

// C types --------------------------------------------------------------------
typedef struct
{
  UInt16         : 14;
  UInt16 gensym  :  1;
  UInt16 keyword :  1;
} AtomFl;

typedef struct Environment
{
  OBJ_HEAD(UInt16);
  struct Environment *parent;

  // function scope -----------------------------------------------------------
  Vector   *local;
  Vector   *upVals;

  // module scope -------------------------------------------------------------
  Map      *vars;
  Map      *syntax;
  Map      *imports;
  Function *owner;
} Environment;

typedef enum __attribute__((packed))
  {
    REF_UNBOUND,   // error, unbound name
    REF_LOCAL,     // value on stack
    REF_CLOSURE,   // value in upvalue
    REF_MODULE,    // value defined at module level
    REF_SYNTAX,    // value refers to a macro
    REF_EXTERN,    // 
  } ReferenceType;

typedef struct
{
  ReferenceType refType;

  UInt8          : 7;
  UInt8 isConst  : 1;
} VarFl;

typedef struct
{
  OBJ_HEAD(VarFl);

  Atom        *name;
  Environment *envt;
  Value        bind;
  List        *props;
} Var;

struct Atom
{
  OBJ_HEAD(AtomFl);

  Table     *ns;
  String    *name;
  Hash       hash;
  Idno       idno;
};

// forward declarations -------------------------------------------------------
Atom *newAtom( const char *name, Table *ns );
Value internAtom( const char *name );

ReferenceType resolveAtom( Atom *atom, Environment *envt, Bool expanded, Var **buf );

// utility macros -------------------------------------------------------------
#define asAtom(x)       asObjType(Atom, x)
#define isAtom(x)       isObjType(x, VAL_ATOM)
#define asVar(x)        asObjType(Var, x)
#define isVar(x)        isObjType(x, VAL_VAR)
#define asEnv(x)        asObjType(Environment, x)
#define isEnv(x)        isObjType(x, VAL_ENVIRONMENT)

#define atomGensym(x)   (asAtom(x)->object.flags.gensym)
#define atomKeyword(x)  (asAtom(x)->object.flags.keyword)
#define atomNs(x)       (asAtom(x)->ns)
#define atomName(x)     (asAtom(x)->name)
#define atomHash(x)     (asAtom(x)->hash)
#define atomIdno(x)     (asAtom(x)->idno)

#define varRefType(x)   (asVar(x)->object.flags.refType)
#define varIsConst(x)   (asVar(x)->object.flags.isConst)
#define varName(x)      (asVar(x)->name)
#define varEnvt(x)      (asVar(x)->envt)
#define varBind(x)      (asVar(x)->bind)
#define varProps(x)     (asVar(x)->props)

#define envParent(x)    (asEnv(x)->parent)
#define envLocal(x)     (asEnv(x)->local)
#define envUpVals(x)    (asEnv(x)->upVals)
#define envVars(x)      (asEnv(x)->module)
#define envSyntax(x)    (asEnv(x)->syntax)
#define envImports(x)   (asEnv(x)->imports)
#define envOwner(x)     (asEnv(x)->owner)

#endif
