#include <string.h>
#include <assert.h>

#include "compare.h"
#include "object.h"
#include "utils.h"

/* globals */
#define LT_VAL (SMALL_TAG|UINT32_MAX)
#define GT_VAL (SMALL_TAG|1)
#define EQ_VAL (SMALL_TAG|0)

#define COMPARE_BOUND 128

/* internal helpers */
static void add_backedges(Table* table, Val a, Val b, Val c, Val cb);
static Val trace_backedge(Table *table, Val k);

static Val equal_reals(Val x, Val y);
static Val equal_fixs(Val x, Val y);
static Val equal_smalls(Val x, Val y);
static Val equal_units(Val x, Val y);
static Val equal_bools(Val x, Val y);
static Val equal_glyphs(Val x, Val y);
static Val equal_syms(Val x, Val y);
static Val equal_ports(Val x, Val y);
static Val equal_native_fns(Val x, Val y);
static Val equal_bins(Val x, Val y);
static Val equal_strs(Val x, Val y);

static Val bounded_equal(Val x, Val y, int bound);
static Val bounded_equal_user_fns(Val x, Val y, int bound);
static Val bounded_equal_pairs(Val x, Val y, int bound);
static Val bounded_equal_lists(Val x, Val y, int bound);
static Val bounded_equal_vecs(Val x, Val y, int bound);
static Val bounded_equal_tables(Val x, Val y, int bound);

static Val cyc_equal(Val x, Val y, Table* table);
static Val cyc_equal_user_fns(Val x, Val y, Table* table);
static Val cyc_equal_pairs(Val x, Val y, Table* table);
static Val cyc_equal_lists(Val x, Val y, Table* table);
static Val cyc_equal_vecs(Val x, Val y, Table* table);
static Val cyc_equal_tables(Val x, Val y, Table* table);

static Val compare_reals(Val x, Val y);
static Val compare_fixs(Val x, Val y);
static Val compare_smalls(Val x, Val y);
static Val compare_units(Val x, Val y);
static Val compare_bools(Val x, Val y);
static Val compare_glyphs(Val x, Val y);
static Val compare_syms(Val x, Val y);
static Val compare_ports(Val x, Val y);
static Val compare_native_fns(Val x, Val y);
static Val compare_bins(Val x, Val y);
static Val compare_strs(Val x, Val y);

static Val bounded_compare(Val x, Val y, int bound);
static Val bounded_compare_user_fns(Val x, Val y, int bound);
static Val bounded_compare_pairs(Val x, Val y, int bound);
static Val bounded_compare_lists(Val x, Val y, int bound);
static Val bounded_compare_vecs(Val x, Val y, int bound);
static Val bounded_compare_tables(Val x, Val y, int bound);

static Val cyc_compare(Val x, Val y, Table* table);
static Val cyc_compare_user_fns(Val x, Val y, Table* table);
static Val cyc_compare_pairs(Val x, Val y, Table* table);
static Val cyc_compare_lists(Val x, Val y, Table* table);
static Val cyc_compare_vecs(Val x, Val y, Table* table);
static Val cyc_compare_tables(Val x, Val y, Table* table);

// cycle detection helpers ----------------------------------------------------
static void add_backedges(Table* table, Val a, Val b, Val c, Val cb) {
  Val ca = c == NIL_VAL ? a : c;

  if (cb != NIL_VAL)
    table_set(table, cb, ca);
  
  table_set(table, a, ca);
  table_set(table, b, ca);
}

static Val trace_backedge(Table* table, Val k) {
  Val b;

  if (!table_ref(table, k, &b))
    return NIL_VAL;

  if (b == k)
    return b;

  return trace_backedge(table, b);
}


/* API */
// same (pointer comparison) --------------------------------------------------
bool same(Val x, Val y) {
  return x == y;
}

// equal ----------------------------------------------------------------------
bool equal(Val x, Val y) {
  Val result = bounded_equal(x, y, COMPARE_BOUND);

  if (result == NIL_VAL) {
    Table cycles;
    init_table(&cycles, true);
    result = cyc_equal(x, y, &cycles);
    free_table(&cycles);
  }

  return as_bool(result);
}

// leaf types -----------------------------------------------------------------
static Val equal_reals(Val x, Val y) { return mk_bool(x == y); }
static Val equal_fixs(Val x, Val y) { return mk_bool(x == y); }
static Val equal_smalls(Val x, Val y) { return mk_bool(x == y); }
static Val equal_units(Val x, Val y) { return mk_bool(x == y); }
static Val equal_bools(Val x, Val y) { return mk_bool(x == y); }
static Val equal_glyphs(Val x, Val y) { return mk_bool(x == y); }
static Val equal_syms(Val x, Val y) { return mk_bool(x == y); }

static Val equal_ports(Val x, Val y) {
  return mk_bool(as_port(x)->ios == as_port(y)->ios);
}

static Val equal_native_fns(Val x, Val y) {
  return mk_bool(as_native_fn(x)->name == as_native_fn(y)->name);
}

static Val equal_bins(Val x, Val y) {
  Bin* xb = as_bin(x), * yb = as_bin(y);

  return mk_bool(xb->count == yb->count && memcmp(xb->array, yb->array, xb->count) == 0);
}

static Val equal_strs(Val x, Val y) {
  Str* xs = as_str(x), * ys = as_str(y);

  return mk_bool(xs->count == ys->count && strcmp(xs->array, ys->array) == 0);
}

// bounded equal --------------------------------------------------------------
static Val bounded_equal(Val x, Val y, int bound) {
  if (x == y)
    return TRUE_VAL;

  if (bound <= 0)
    return NIL_VAL;

  ValType xv = val_type(x), yv = val_type(y);

  if (xv != yv)
    return FALSE_VAL;

  switch (xv) {
    case REAL_VAL:  return equal_reals(x, y);
    case FIX_VAL:   return equal_fixs(x, y);
    case SMALL_VAL: return equal_smalls(x, y);
    case UNIT_VAL:  return equal_units(x, y);
    case BOOL_VAL:  return equal_bools(x, y);
    case GLYPH_VAL: return equal_glyphs(x, y);
    case OBJ_VAL: {
      ObjType xo = obj_type(x), yo = obj_type(y);
      
      if (xo != yo)
	return FALSE_VAL;
      
      switch (xo) {
	case SYM_OBJ:       return equal_syms(x, y);
	case PORT_OBJ:      return equal_ports(x, y);
	case NATIVE_FN_OBJ: return equal_native_fns(x, y);
	case BIN_OBJ:       return equal_bins(x, y);
	case STR_OBJ:       return equal_strs(x, y);
	case USER_FN_OBJ:   return bounded_equal_user_fns(x, y, bound);
	case PAIR_OBJ:      return bounded_equal_pairs(x, y, bound);
	case LIST_OBJ:      return bounded_equal_lists(x, y, bound);
	case VEC_OBJ:       return bounded_equal_vecs(x, y, bound);
	case TABLE_OBJ:     return bounded_equal_tables(x, y, bound);
      }
    }
  }
  
  unreachable();
}

static Val bounded_equal_user_fns(Val x, Val y, int bound) {
  Val a, b, c;

  UserFn* xf = as_user_fn(x), * yf = as_user_fn(y);

  if (xf->name != yf->name)
    return FALSE_VAL;

  if (xf->n_args != yf->n_args)
    return FALSE_VAL;

  if (xf->vargs != yf->vargs)
    return FALSE_VAL;

  a = mk_obj((Obj*)xf->ns); b = mk_obj((Obj*)yf->ns);

  if ((c=bounded_equal(a, b, bound-1)) != TRUE_VAL)
    return c;

  a = mk_obj((Obj*)xf->env); b = mk_obj((Obj*)yf->env);

  if ((c=bounded_equal(a, b, bound-1)) != TRUE_VAL)
    return c;

  a = mk_obj((Obj*)xf->consts); b = mk_obj((Obj*)yf->consts);

  if ((c=bounded_equal(a, b, bound-1)) != TRUE_VAL)
    return c;

  a = mk_obj((Obj*)xf->code); b = mk_obj((Obj*)yf->code);

  return equal_bins(a, b);
}

static Val bounded_equal_pairs(Val x, Val y, int bound) {
  Val c;

  Pair* xp = as_pair(x), * yp = as_pair(y);

  if ((c=bounded_equal(xp->fst, yp->fst, bound-1)) != TRUE_VAL)
    return c;

  return bounded_equal(xp->snd, yp->snd, bound-1);
}

static Val bounded_equal_lists(Val x, Val y, int bound) {
  Val c;

  List* xs = as_list(x), * ys = as_list(y);

  if (xs->length != ys->length)
    return FALSE_VAL;

  while (xs->length) {
    if ((c=bounded_equal(xs->head, ys->head, bound-1)) != TRUE_VAL)
      return c;

    xs = xs->tail;
    ys = ys->tail;
  }

  return TRUE_VAL;
}

static Val bounded_equal_vecs(Val x, Val y, int bound) {
  Val c;

  Vec* xs = as_vec(x), * ys = as_vec(y);

  if (xs->count != ys->count)
    return FALSE_VAL;

  for (int i=0; i<xs->count; i++) {
    if ((c=bounded_compare(xs->array[i], ys->array[i], bound-1)) != TRUE_VAL)
      return c;
  }

  return TRUE_VAL;
}

static Val bounded_equal_tables(Val x, Val y, int bound) {
  Val c;

  Table* xt = as_table(x), * yt = as_table(y);

  if (xt->count != yt->count)
    return FALSE_VAL;

  for (int i=0; i<xt->count; i++) {
    Pair* xp = xt->table[i], * yp = yt->table[i];

    if ((c=bounded_compare(xp->fst, yp->fst, bound-1)) != TRUE_VAL)
      return c;

    if ((c=bounded_compare(xp->snd, yp->snd, bound-1)) != TRUE_VAL)
      return c;
  }

  return TRUE_VAL;
}

// cyc equal ---------------------------------------------------------------
static Val cyc_equal(Val x, Val y, Table* table) {
  if (x == y)
    return TRUE_VAL;

  ValType xv = val_type(x), yv = val_type(y);

  if (xv != yv)
    return FALSE_VAL;

  switch (xv) {
    case REAL_VAL:  return equal_reals(x, y);
    case FIX_VAL:   return equal_fixs(x, y);
    case SMALL_VAL: return equal_smalls(x, y);
    case UNIT_VAL:  return equal_units(x, y);
    case BOOL_VAL:  return equal_bools(x, y);
    case GLYPH_VAL: return equal_glyphs(x, y);
    case OBJ_VAL: {
      ObjType xo = obj_type(x), yo = obj_type(y);
      
      if (xo != yo)
	return FALSE_VAL;

      switch (xo) {
	case SYM_OBJ:       return equal_syms(x, y);
	case PORT_OBJ:      return equal_ports(x, y);
	case NATIVE_FN_OBJ: return equal_native_fns(x, y);
	case BIN_OBJ:       return equal_bins(x, y);
	case STR_OBJ:       return equal_strs(x, y);
	case USER_FN_OBJ:   return cyc_equal_user_fns(x, y, table);
	case PAIR_OBJ:      return cyc_equal_pairs(x, y, table);
	case LIST_OBJ:      return cyc_equal_lists(x, y, table);
	case VEC_OBJ:       return cyc_equal_vecs(x, y, table);
	case TABLE_OBJ:     return cyc_equal_tables(x, y, table);
      }
    }
  }
  
  unreachable();
}

static Val cyc_equal_user_fns(Val x, Val y, Table* table) {
  Val z, xb = trace_backedge(table, x), yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return TRUE_VAL;

  add_backedges(table, x, y, xb, yb);

  UserFn* xf = as_user_fn(x), * yf = as_user_fn(y);

  if (xf->name != yf->name)
    return FALSE_VAL;

  if (xf->n_args != yf->n_args)
    return FALSE_VAL;

  if (xf->vargs != yf->vargs)
    return FALSE_VAL;

  x = mk_obj((Obj*)xf->ns); y = mk_obj((Obj*)yf->ns);

  if ((z=cyc_compare(x, y, table)) != TRUE_VAL)
    return z;

  x = mk_obj((Obj*)xf->env); y = mk_obj((Obj*)yf->env);

  if ((z=cyc_compare(x, y, table)) != TRUE_VAL)
    return z;

  x = mk_obj((Obj*)xf->consts); y = mk_obj((Obj*)yf->consts);

  if ((z=cyc_compare(x, y, table)) != TRUE_VAL)
    return z;

  x = mk_obj((Obj*)xf->consts); y = mk_obj((Obj*)yf->consts);

  return cyc_compare(x, y, table);
}

static Val cyc_equal_pairs(Val x, Val y, Table* table) {
  Val z;

  Pair* xp = as_pair(x), * yp = as_pair(y);
  
  Val xb = trace_backedge(table, x);
  Val yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return TRUE_VAL;

  add_backedges(table, x, y, xb, yb);

  if ((z=cyc_equal(xp->fst, yp->fst, table)) != TRUE_VAL)
    return z;

  return cyc_equal(xp->snd, yp->snd, table);
}

static Val cyc_equal_lists(Val x, Val y, Table* table) {
  Val z, xb, yb;
  List* xs = as_list(x),* ys = as_list(y);

  if (xs->length != ys->length)
    return FALSE_VAL;

  while (xs->length) {
    
    xb = trace_backedge(table, x);
    yb = trace_backedge(table, y);

    if (xb != NIL_VAL && xb==yb)
      return TRUE_VAL;

    add_backedges(table, x, y, xb, yb);

    if ((z=cyc_equal(xs->head, ys->head, table)) != TRUE_VAL)
      return z;

    xs = xs->tail; x = mk_obj((Obj*)xs);
    ys = ys->tail; y = mk_obj((Obj*)ys);
  }

  return TRUE_VAL;
}

static Val cyc_equal_vecs(Val x, Val y, Table* table) {
  Val z, xb, yb;

  Vec* xs = as_vec(x), * ys = as_vec(y);

  if (xs->count != ys->count)
    return FALSE_VAL;

  xb = trace_backedge(table, x);
  yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return TRUE_VAL;

  add_backedges(table, x, y, xb, yb);

  for (int i=0; i<xs->count; i++) {
    if ((z=cyc_equal(xs->array[i], ys->array[i], table)) != TRUE_VAL)
      return z;
  }

  return TRUE_VAL;
}

static Val cyc_equal_tables(Val x, Val y, Table* table) {
  Val z, xb, yb;

  Table* xt = as_table(x), * yt = as_table(y);

  if (xt->count != yt->count)
    return FALSE_VAL;

  xb = trace_backedge(table, x);
  yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return TRUE_VAL;

  for (int i=0; i<xt->count; i++) {
    Pair* xp = xt->table[i], * yp = yt->table[i];

    if ((z=cyc_equal(xp->fst, yp->fst, table)) != TRUE_VAL)
      return z;

    if ((z=cyc_equal(xp->snd, yp->snd, table)) != TRUE_VAL)
      return z;
  }

  return TRUE_VAL;
}

// compare --------------------------------------------------------------------
int compare(Val x, Val y) {
  Val result = bounded_compare(x, y, COMPARE_BOUND);

  if (result == NIL_VAL) {
    Table cycles;
    init_table(&cycles, true);
    result = cyc_compare(x, y, &cycles);
    free_table(&cycles);
  }

  return as_small(result);
}

// leaf types -----------------------------------------------------------------
Val compare_reals(Val x, Val y) {
  return mk_small(cmp_reals(as_real(x), as_real(y)));
}

Val compare_fixs(Val x, Val y) {
  return mk_small(cmp_words(as_real(x), as_real(y)));
}

Val compare_smalls(Val x, Val y) {
  return mk_small(cmp_ints(as_small(x), as_small(y)));
}

Val compare_units(Val x, Val y) {
  return mk_small(cmp_words(x&VAL_MASK, y&VAL_MASK));
}

Val compare_bools(Val x, Val y) {
  return mk_small(as_bool(x) - as_bool(y));
}

Val compare_glyphs(Val x, Val y) {
  return mk_small(cmp_ints(as_glyph(x), as_glyph(y)));
}

Val compare_syms(Val x, Val y) {
  if (x == y)
    return EQ_VAL;

  Sym* xs = as_sym(x), * ys = as_sym(y);

  int o = strcmp(xs->name, ys->name);

  return mk_small(o ? : cmp_words(xs->idno, ys->idno));
}

Val compare_ports(Val x, Val y) {
  return mk_small(cmp_words((word)as_port(x)->ios, (word)as_port(y)->ios));
}

Val compare_native_fns(Val x, Val y) {
  NativeFn* xn = as_native_fn(x), * yn = as_native_fn(y);

  if (xn->name == yn->name)
    return EQ_VAL;

  int o = strcmp(xn->name->name, yn->name->name);

  return mk_small(o ? : cmp_words(xn->name->idno, yn->name->idno));
}

Val compare_bins(Val x, Val y) {
  Bin* xb = as_bin(x), * yb = as_bin(y);
  int max_cmp = MIN(xb->count, yb->count);
  int o = memcmp(xb->array, yb->array, max_cmp);

  return mk_small(o ? : cmp_ints(xb->count, yb->count));
}

Val compare_strs(Val x, Val y) {
  Str* xs = as_str(x), * ys = as_str(y);
  int max_cmp = MIN(xs->count, ys->count);
  int o = strncmp(xs->array, ys->array, max_cmp);

  return mk_small(o ? : cmp_ints(xs->count, ys->count));
}

// bounded compare ------------------------------------------------------------
Val bounded_compare(Val x, Val y, int bound) {
  if (x == y)
    return EQ_VAL;

  if (bound <= 0)
    return NIL_VAL;

  ValType xv = val_type(x), yv = val_type(y);

  if (xv != yv)
    return mk_small(cmp_ints(xv, yv));

  switch (xv) {
    case REAL_VAL:  return compare_reals(x, y);
    case FIX_VAL:   return compare_fixs(x, y);
    case SMALL_VAL: return compare_smalls(x, y);
    case UNIT_VAL:  return compare_units(x, y);
    case BOOL_VAL:  return compare_bools(x, y);
    case GLYPH_VAL: return compare_glyphs(x, y);
    case OBJ_VAL: {
      ObjType xo = obj_type(x), yo = obj_type(y);

      if (xo != yo)
	return mk_small(cmp_ints(xo, yo));

      switch (xo) {
	case SYM_OBJ:       return compare_syms(x, y);
	case PORT_OBJ:      return compare_ports(x, y);
	case NATIVE_FN_OBJ: return compare_native_fns(x, y);
	case BIN_OBJ:       return compare_bins(x, y);
	case STR_OBJ:       return compare_strs(x, y);
	case USER_FN_OBJ:   return bounded_compare_user_fns(x, y, bound);
	case PAIR_OBJ:      return bounded_compare_pairs(x, y, bound);
	case LIST_OBJ:      return bounded_compare_lists(x, y, bound);
	case VEC_OBJ:       return bounded_compare_vecs(x, y, bound);
	case TABLE_OBJ:     return bounded_compare_tables(x, y, bound);
      }
    }
  }

  unreachable();
}


static Val bounded_compare_user_fns(Val x, Val y, int bound) {
  Val a, b, c;

  UserFn* xf = as_user_fn(x), * yf = as_user_fn(y);

  a = mk_obj((Obj*)xf->name); b = mk_obj((Obj*)yf->name);

  if ((c=bounded_compare(a, b, bound-1)) != EQ_VAL)
    return c;

  if (xf->n_args != yf->n_args)
    return mk_small(cmp_ints(xf->n_args, yf->n_args));

  if (xf->vargs != yf->vargs)
    return mk_small(cmp_ints(xf->vargs, yf->vargs));

  a = mk_obj((Obj*)xf->ns); b = mk_obj((Obj*)yf->ns);

  if ((c=bounded_compare(a, b, bound-1)) != EQ_VAL)
    return c;

  a = mk_obj((Obj*)xf->env); b = mk_obj((Obj*)yf->env);

  if ((c=bounded_compare(a, b, bound-1)) != EQ_VAL)
    return c;

  a = mk_obj((Obj*)xf->consts); b = mk_obj((Obj*)yf->consts);

  if ((c=bounded_compare(a, b, bound-1)) != EQ_VAL)
    return c;

  a = mk_obj((Obj*)xf->code); b = mk_obj((Obj*)yf->code);

  return compare_bins(a, b);
}

static Val bounded_compare_pairs(Val x, Val y, int bound) {
  Val c;

  Pair* xp = as_pair(x), * yp = as_pair(y);

  if ((c=bounded_compare(xp->fst, yp->fst, bound-1)) != EQ_VAL)
    return c;

  return bounded_compare(xp->snd, yp->snd, bound-1);
}

static Val bounded_compare_lists(Val x, Val y, int bound) {
  Val c;
  List* xs = as_list(x), * ys = as_list(y);
  int max_cmp = MIN(xs->length, ys->length);

  for (int i=0; i<max_cmp; i++, xs=xs->tail, ys=ys->tail) {
    if ((c=bounded_compare(xs->head, ys->head, bound-1)) != EQ_VAL)
      return c;
  }

  return mk_small(xs->length - ys->length);
}

static Val bounded_compare_vecs(Val x, Val y, int bound) {
  Val c;
  Vec* xs = as_vec(x), * ys = as_vec(y);
  int max_cmp = MIN(xs->count, ys->count);

  for (int i=0; i<max_cmp; i++) {
    if ((c=bounded_compare(xs->array[i], ys->array[i], bound-1)) != EQ_VAL)
      return c;
  }

  return mk_small(xs->count-ys->count);
}

static Val bounded_compare_tables(Val x, Val y, int bound) {
  Val c;
  Table* xt = as_table(x), * yt = as_table(y);
  int max_cmp = MIN(xt->count, yt->count);

  for (int i=0; i<max_cmp; i++) {
    Pair* xp = xt->table[i], * yp = yt->table[i];

    if ((c=bounded_compare(xp->fst, yp->fst, bound-1)) != EQ_VAL)
      return c;

    if ((c=bounded_compare(xp->snd, yp->snd, bound-1)) != EQ_VAL)
      return c;
  }

  return mk_small(xt->count - yt->count);
}

// cyc compare ----------------------------------------------------------------
static Val cyc_compare(Val x, Val y, Table* table) {
  if (x == y)
    return EQ_VAL;

  ValType xv = val_type(x), yv = val_type(y);

  if (xv != yv)
    return mk_small(xv-yv);

  switch (xv) {
    case REAL_VAL:  return compare_reals(x, y);
    case FIX_VAL:   return compare_fixs(x, y);
    case SMALL_VAL: return compare_smalls(x, y);
    case UNIT_VAL:  return compare_units(x, y);
    case BOOL_VAL:  return compare_bools(x, y);
    case GLYPH_VAL: return compare_glyphs(x, y);
    case OBJ_VAL: {
      ObjType xo = obj_type(x), yo = obj_type(y);
      
      if (xo != yo)
	return mk_small(xo-yo);

      switch (xo) {
	case SYM_OBJ:       return compare_syms(x, y);
	case PORT_OBJ:      return compare_ports(x, y);
	case NATIVE_FN_OBJ: return compare_native_fns(x, y);
	case BIN_OBJ:       return compare_bins(x, y);
	case STR_OBJ:       return compare_strs(x, y);
	case USER_FN_OBJ:   return cyc_compare_user_fns(x, y, table);
	case PAIR_OBJ:      return cyc_compare_pairs(x, y, table);
	case LIST_OBJ:      return cyc_compare_lists(x, y, table);
	case VEC_OBJ:       return cyc_compare_vecs(x, y, table);
	case TABLE_OBJ:     return cyc_compare_tables(x, y, table);
      }
    }
  }
  
  unreachable();
}

static Val cyc_compare_user_fns(Val x, Val y, Table* table) {
  Val z, xb = trace_backedge(table, x), yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return EQ_VAL;

  add_backedges(table, x, y, xb, yb);

  UserFn* xf = as_user_fn(x), * yf = as_user_fn(y);

  x = mk_obj((Obj*)xf->name); y = mk_obj((Obj*)yf->name);

  if ((z=compare_syms(x, y)))
    return z;

  if (xf->n_args != yf->n_args)
    return mk_small(xf->n_args-yf->n_args);

  if (xf->vargs != yf->vargs)
    return mk_small(xf->vargs-yf->vargs);

  x = mk_obj((Obj*)xf->ns); y = mk_obj((Obj*)yf->ns);

  if ((z=cyc_compare(x, y, table)) != EQ_VAL)
    return z;

  x = mk_obj((Obj*)xf->env); y = mk_obj((Obj*)yf->env);

  if ((z=cyc_compare(x, y, table)) != EQ_VAL)
    return z;

  x = mk_obj((Obj*)xf->consts); y = mk_obj((Obj*)yf->consts);

  if ((z=cyc_compare(x, y, table)) != EQ_VAL)
    return z;

  x = mk_obj((Obj*)xf->consts); y = mk_obj((Obj*)yf->consts);

  return cyc_compare(x, y, table);
}

static Val cyc_compare_pairs(Val x, Val y, Table* table) {
  Val z;

  Pair* xp = as_pair(x), * yp = as_pair(y);
  
  Val xb = trace_backedge(table, x);
  Val yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return EQ_VAL;

  add_backedges(table, x, y, xb, yb);

  if ((z=cyc_compare(xp->fst, yp->fst, table)) != EQ_VAL)
    return z;

  return cyc_compare(xp->snd, yp->snd, table);
}

static Val cyc_compare_lists(Val x, Val y, Table* table) {
  Val z, xb, yb;
  List* xs = as_list(x),* ys = as_list(y);
  int max_cmp = MIN(xs->length, ys->length);

  for (int i=0; i<max_cmp; i++,xs=xs->tail,ys=ys->tail,x=mk_obj((Obj*)xs),y=mk_obj((Obj*)ys)) {
    xb = trace_backedge(table, x);
    yb = trace_backedge(table, y);

    if (xb != NIL_VAL && xb==yb)
      return EQ_VAL;

    add_backedges(table, x, y, xb, yb);

    if ((z=cyc_compare(xs->head, ys->head, table)) != EQ_VAL)
      return z;
  }

  return mk_small(xs->length-ys->length);
}

static Val cyc_compare_vecs(Val x, Val y, Table* table) {
  Val z, xb, yb;

  Vec* xs = as_vec(x), * ys = as_vec(y);
  int max_cmp = MIN(xs->count, ys->count);

  xb = trace_backedge(table, x);
  yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return EQ_VAL;

  add_backedges(table, x, y, xb, yb);

  for (int i=0; i<max_cmp; i++) {
    if ((z=cyc_compare(xs->array[i], ys->array[i], table)) != EQ_VAL)
      return z;
  }

  return mk_small(xs->count-ys->count);
}

static Val cyc_compare_tables(Val x, Val y, Table* table) {
  Val z, xb, yb;

  Table* xt = as_table(x), * yt = as_table(y);
  int max_cmp = MIN(xt->count, yt->count);

  xb = trace_backedge(table, x);
  yb = trace_backedge(table, y);

  if (xb != NIL_VAL && xb==yb)
    return EQ_VAL;

  for (int i=0; i<max_cmp; i++) {
    Pair* xp = xt->table[i], * yp = yt->table[i];

    if ((z=cyc_compare(xp->fst, yp->fst, table)) != EQ_VAL)
      return z;

    if ((z=cyc_compare(xp->snd, yp->snd, table)) != EQ_VAL)
      return z;
  }

  return mk_small(xt->count-yt->count);
}
