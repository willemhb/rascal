#include "vm/memory.h"

#include "val/map.h"
#include "val/vector.h"
#include "val/func.h"
#include "val/symbol.h"
#include "val/type.h"
#include "val/text.h"
#include "val/list.h"
#include "val/seq.h"

/* Internal APIs */
static void init_iseq(void* obj, size_t offset, Obj* nested, Obj* src) {
  ISeq* seq = obj;
  
  seq->seq.nested = nested;
  seq->seq.offset = offset;
  seq->seq.first  = NOTHING;
  seq->src        = src;
}

static void advance(void* obj) {
  ISeq* seq = obj;

  seq->seq.offset++;
  seq->seq.first = NOTHING;
}

static void* mk_iseq(Type* type, bool editp, Obj* src) {
  save(2, tag(type), tag(src));
  ISeq* out = out = new_obj(type, editp*EDITP, 0, 0);
  init_iseq(out, 0, NULL, src);

  return out;
}

static bool is_editp(void* obj) {
  return get_mfl(obj, EDITP);
}

void trace_iseq(void* obj) {
  ISeq* seq = obj;

  mark(seq->src);
  mark(seq->seq.nested);
}

#define INIT_ISEQ_TYPE(_T)                      \
  INIT_OBJECT_TYPE(_T, NULL, trace_iseq, NULL)

/* External APIs */
/* Sequence interface for Lists. */
ListSeq* list_mk_seq(List* obj, bool editp) {
  (void)editp;

  ListSeq* out = obj && obj->arity > 0 ? obj : NULL;
  return out;
}

bool list_seq_has_next(ListSeq* obj) {
  return obj && obj->arity > 1;
}

Value list_seq_first(ListSeq* obj) {
  if (obj == NULL)
    return NOTHING;
  
  return obj->head;
}

ListSeq* list_seq_rest(ListSeq* obj) {
  ListSeq* out = obj && obj->arity > 1 ? obj->tail : obj;

  return out;
}

/* Sequence interface for Strings */
INIT_ISEQ_TYPE(String);

StringSeq* str_mk_seq(String* obj, bool editp) {
  StringSeq* out = NULL;

  if (obj->arity > 0)
    out = mk_iseq(&StringSeqType, editp, (Obj*)obj);

  return out;
}

bool str_seq_has_next(StringSeq* seq) {
  if (seq == NULL)
    return false;

  return seq->seq.offset + 1 < seq->src->arity;
}

Value str_seq_first(StringSeq* seq) {
  if (seq == NULL)
    return NOTHING;

  if (seq->seq.first == NOTHING)
    seq->seq.first = tag((Glyph)seq->src->data[seq->seq.offset]);

  return seq->seq.first;
}

StringSeq* str_seq_rest(StringSeq* seq) {
  if (!has_next(seq))
    return NULL;

  if (!is_editp(seq))
    seq = clone_obj(seq);

  advance(seq);
  return seq;
}

/* Sequence interface for Vectors */
INIT_ISEQ_TYPE(Vector);

VectorSeq* vec_mk_seq(Vector* obj, bool editp) {
  VectorSeq* out = NULL;

  if (obj && obj->arity > 0)
    out = mk_iseq(&VectorSeqType, editp, (Obj*)obj);

  return out;
}

bool vec_seq_has_next(VectorSeq* seq) {
  return seq && seq->seq.offset + 1 < seq->src->arity;
}

Value vec_seq_first(VectorSeq* seq) {
  if (seq == NULL)
    return NOTHING;

  if (seq->seq.first == NOTHING)
    seq->seq.first = vec_ref(seq->src, seq->seq.offset);

  return seq->seq.first;
}

VectorSeq* vec_seq_rest(VectorSeq* seq) {
  VectorSeq* out = NULL;

  if (has_next(seq)) {
    if (!is_editp(seq))
      seq = clone_obj(seq);

    advance(seq);
  }

  return out;
}

/* DictSeq type */
INIT_ISEQ_TYPE(Dict);

DictSeq* dict_mk_seq(Dict* obj, bool editp) {
  DictSeq* out = NULL;

  if (obj && obj->arity > 0) {
    out = mk_iseq(&DictSeqType, editp, (Obj*)obj);

    save(1, tag(out));

    out->seq.nested = (Obj*)mk_seq(obj->root, editp);
  }

  return out;
}

bool dict_seq_has_next(DictSeq* seq) {
  return seq && has_next((DictNodeSeq*)seq->seq.nested);
}

Value dict_seq_first(DictSeq* seq) {
  if (seq == NULL)
    return NOTHING;

  if (seq->seq.first == NOTHING)
    seq->seq.first = first((DictNodeSeq*)seq->seq.nested);

  return seq->seq.first;
}

DictSeq* dict_seq_rest(DictSeq* seq) {
  DictSeq* out = NULL;
  
  if (has_next(seq)) {
    if (!is_editp(seq))
      seq = clone_obj(seq);

    save(1, tag(seq));

    seq->seq.nested = (Obj*)rest((DictNodeSeq*)seq);
    seq->seq.offset++;
    seq->seq.first  = NOTHING;
  }

  return out;
}

/* DictNodeSeq interface */
INIT_ISEQ_TYPE(DictNode);

DictNodeSeq* dict_node_mk_seq(DictNode* obj, bool editp) {
  DictNodeSeq* seq = NULL;
  if (obj != NULL) {
    seq = mk_iseq(&DictNodeSeqType, editp, (Obj*)obj);

    save(1, tag(seq));

    Obj* first_child = obj->data[0];

    if (first_child->type == &DictNodeType)
      seq->seq.nested = (Obj*)mk_seq((DictNode*)first_child, editp);

    else
      seq->seq.nested = first_child;
  }

  return seq;
}

bool dict_node_seq_has_next(DictNodeSeq* seq) {
  bool out = false;

  if (seq != NULL) {
    if (seq->seq.nested->type == &DictLeafType)
      out = has_next((DictLeaf*)seq->seq.nested);
    
    else
      out = has_next((DictNodeSeq*)seq->seq.nested);

    if (out == false)
      out = seq->seq.offset + 1 < get_hamt_cnt(seq->src);
  }

  return out;
}

Value dict_node_seq_first(DictNodeSeq* seq) {
  Value out = NOTHING;

  if (seq != NULL) {
    if (seq->seq.first == NOTHING) {
      if (seq->seq.nested->type == &DictLeafType)
        seq->seq.first = first((DictLeaf*)seq->seq.nested);

      else
        seq->seq.first = first((DictNodeSeq*)seq->seq.nested);
    }
    out = seq->seq.first;
  }

  return out;
}

DictNodeSeq* dict_node_seq_rest(DictNodeSeq* seq) {
  DictNodeSeq* out = NULL;

  if (has_next(seq)) {
    bool editp = is_editp(seq);

    if (!editp)
      seq = clone_obj(seq);

    save(1, tag(seq));

    if (seq->seq.nested->type == &DictLeafType)
      seq->seq.nested = (Obj*)rest((DictLeaf*)seq->seq.nested);

    else
      seq->seq.nested = (Obj*)rest((DictNodeSeq*)seq->seq.nested);

    if (seq->seq.nested == NULL) {
      seq->seq.offset++;
      Obj* next_src = seq->src->data[seq->seq.offset];

      if (next_src->type == &DictLeafType)
        seq->seq.nested = next_src;

      else
        seq->seq.nested = (Obj*)mk_seq((DictNode*)next_src, editp);
    }

    seq->seq.first = NOTHING;
    out = seq;
  }

  return out;
}

/* DictLeaf sequence interface */
DictLeafSeq* dict_leaf_mk_seq(DictLeaf* obj, bool editp) {
  (void)editp;

  return obj;
}

bool dict_leaf_seq_has_next(DictLeafSeq* seq) {
  return seq && seq->next;
}

Value dict_leaf_seq_first(DictLeafSeq* seq) {
  if (seq == NULL)
    return NOTHING;

  return tag(seq);
}

DictLeafSeq* dict_leaf_seq_rest(DictLeafSeq* seq) {
  DictLeafSeq* out = NULL;

  if (seq)
    out = seq->next;

  return out;
}

/* Set sequence interface */
INIT_ISEQ_TYPE(Set);

SetSeq* set_mk_seq(Set* obj, bool editp) {
  SetSeq* out = NULL;

  if (obj && obj->arity > 0) {
    out = mk_iseq(&SetSeqType, editp, (Obj*)obj);

    save(1, tag(out));

    out->seq.nested = (Obj*)mk_seq(obj->root, editp);
  }

  return out;
}

bool set_seq_has_next(SetSeq* seq) {
  return seq && has_next((SetNodeSeq*)seq->seq.nested);
}

Value seq_set_first(SetSeq* seq) {
  Value out = NOTHING;

  if (seq != NULL) {
    if (seq->seq.first == NOTHING)
      seq->seq.first = first((SetNodeSeq*)seq->seq.nested);

    out = seq->seq.first;
  }

  return out;
}

SetSeq* set_seq_rest(SetSeq* seq) {
  SetSeq* out = NULL;

  if (has_next(seq)) {
    if (!is_editp(seq))
      seq = clone_obj(seq);

    save(1, tag(seq));

    seq->seq.nested = (Obj*)rest((SetNodeSeq*)seq);
    seq->seq.offset++;
    seq->seq.first = NOTHING;
  }

  return out;
}

/* SetNodeSeq interface. */
INIT_ISEQ_TYPE(SetNode);

SetNodeSeq* set_node_mk_seq(SetNode* obj, bool editp) {
  SetNodeSeq* seq = NULL;

  if (obj != NULL) {
    seq = mk_iseq(&SetNodeSeqType, editp, (Obj*)obj);

    save(1, tag(seq));

    Obj* first_child = obj->data[0];

    if (first_child->type == &SetNodeType)
      seq->seq.nested = (Obj*)mk_seq((SetNode*)first_child, editp);

    else
      seq->seq.nested = first_child;
  }

  return seq;
}

bool set_node_seq_has_next(SetNodeSeq* seq) {
  bool out = false;

  if (seq != NULL) {
    if (seq->seq.nested->type == &SetLeafType)
      out = has_next((SetLeaf*)seq->seq.nested);

    else
      out = has_next((SetNodeSeq*)seq->seq.nested);

    if (out == false)
      out = seq->seq.offset + 1 < get_hamt_cnt(seq->src);
  }

  return out;
}

Value set_node_seq_first(SetNodeSeq* seq) {
  Value out = NOTHING;

  if (seq != NULL) {
    if (seq->seq.first == NOTHING) {
      if (seq->seq.nested->type == &SetLeafType)
        seq->seq.first = first((SetLeaf*)seq->seq.nested);

      else
        seq->seq.first = first((SetNodeSeq*)seq->seq.nested);
    }

    out = seq->seq.first;
  }

  return out;
}

SetNodeSeq* set_node_seq_rest(SetNodeSeq* seq) {
  SetNodeSeq* out = NULL;

  if (has_next(seq)) {
    bool editp = is_editp(seq);

    if (!editp)
      seq = clone_obj(seq);

    save(1, tag(seq));

    if (seq->seq.nested->type == &SetLeafType)
      seq->seq.nested = (Obj*)rest((SetLeaf*)seq->seq.nested);

    else
      seq->seq.nested = (Obj*)rest((SetNodeSeq*)seq->seq.nested);

    if (seq->seq.nested == NULL) {
      seq->seq.offset++;
      Obj* next_src = seq->src->data[seq->seq.offset];

      if (next_src->type == &SetLeafType)
        seq->seq.nested = next_src;

      else
        seq->seq.nested = (Obj*)mk_seq((SetNode*)next_src, editp);
    }

    seq->seq.first = NOTHING;
    out = seq;
  }

  return out;
}

/* SetLeaf sequence interface */
SetLeafSeq* set_leaf_mk_seq(SetLeaf* obj, bool editp) {
  (void)editp;

  return obj;
}

bool set_leaf_seq_has_next(SetLeafSeq* seq) {
  return seq && seq->next;
}

Value set_leaf_seq_first(SetLeafSeq* seq) {
  if (seq == NULL)
    return NOTHING;

  return seq->val;
}

SetLeafSeq* set_leaf_seq_rest(SetLeafSeq* seq) {
  SetLeafSeq* out = NULL;

  if (seq)
    out = seq->next;

  return out;
}
