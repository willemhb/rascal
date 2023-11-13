#ifndef rl_vm_seq_h
#define rl_vm_seq_h

#include "val/hamt.h"

/* Sequence interface objects for builtin types. */

/* C types & external APIs */
typedef struct Seq  Seq;
typedef struct ISeq ISeq;

struct Seq {
  HEADER;
  Obj*   nested;
  size_t offset;
  Value  first;
};

struct ISeq {
  Seq  seq;
  Obj* src;
};

#define SEQ_IFACE(_T, _t)                             \
  _T##Seq* _t##_mk_seq(_T* obj, bool editp);          \
  bool     _t##_seq_has_next(_T##Seq* seq);           \
  Value    _t##_seq_first(_T##Seq* seq);              \
  _T##Seq* _t##_seq_rest(_T##Seq* seq)

#define SEQ_TYPE(_T, _t)                        \
  typedef _T _T##Seq;                           \
                                                \
  SEQ_IFACE(_T, _t)

#define ISEQ_TYPE(_T, _t)                             \
  typedef struct _T##Seq _T##Seq;                     \
                                                      \
  struct _T##Seq {                                    \
    Seq seq;                                          \
    _T* src;                                          \
  };                                                  \
                                                      \
  extern Type _T##SeqType;                            \
                                                      \
  SEQ_IFACE(_T, _t)

#define SEQ_METHOD(_m, obj, args...)            \
  generic((obj),                                \
          Vector*:vec_##_m,                     \
          Dict*:dict_##_m,                      \
          DictNode*:dict_node_##_m,             \
          DictLeaf*:dict_leaf_##_m,             \
          Set*:set_##_m,                        \
          SetNode*:set_node_##_m,               \
          SetLeaf*:set_leaf_##_m,               \
          List*:list_##_m,                      \
          String*:str_##_m)(args)

#define SEQ_IMETHOD(_m, obj, args...)                      \
  generic((obj),                                           \
          VectorSeq*:vec_seq_##_m,                         \
          DictSeq*:dict_seq_##_m,                          \
          DictNodeSeq*:dict_node_seq_##_m,                 \
          DictLeafSeq*:dict_leaf_seq_##_m,                 \
          SetSeq*:set_seq_##_m,                            \
          SetNodeSeq*:set_node_seq_##_m,                   \
          SetLeafSeq*:set_leaf_seq_##_m,                   \
          ListSeq*:list_seq_##_m,                          \
          StringSeq*:str_seq_##_m)(args)

SEQ_TYPE(List, list);
SEQ_TYPE(DictLeaf, dict_leaf);
SEQ_TYPE(SetLeaf, set_leaf);

ISEQ_TYPE(Vector, vec);
ISEQ_TYPE(Dict, dict);
ISEQ_TYPE(DictNode, dict_node);
ISEQ_TYPE(Set, set);
ISEQ_TYPE(SetNode, set_node);
ISEQ_TYPE(String, str);

#define mk_seq(obj, editp) SEQ_METHOD(mk_seq, obj, obj, editp)
#define has_next(obj)      SEQ_IMETHOD(has_next, obj, obj)
#define first(obj)         SEQ_IMETHOD(first, obj, obj)
#define rest(obj)          SEQ_IMETHOD(rest, obj, obj)

/* initialize */

#endif
