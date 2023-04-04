#include "internal.h"



// table ----------------------------------------------------------------------
// globals --------------------------------------------------------------------
#define MIN_CAP    8ul
#define MAX_CAP    0x0000800000000000ul
#define TABLE_LOAD 0.625

void print_table(value_t x, port_t* ios);
bool equal_tables(void* px, void* py);
int  compare_tables(value_t x, value_t y);
int  init_table(void* ptr, void* ini);
void trace_table(void* ptr);
void free_table(void* ptr);

data_type_t TableType = {
  .type={
    .obj={
      .type  =DATA_TYPE,
      .frozen=true,
      .gray  =true
    },
    .name="table",
    .idno=TABLE
  },
  .size   =sizeof(table_t),
  .print  =print_table,
  .equal  =equal_tables,
  .compare=compare_tables,
  .init   =init_table,
  .trace  =trace_table,
  .free   =free_table
};

// utilities ------------------------------------------------------------------
usize pad_alist_size(usize newct, usize oldct, usize oldcap, usize mincap) {
  /* CPython resize algorithm */
  usize newcap = MAX(oldcap, mincap);

  if (newcap >= newct && (newct >= (newcap >> 1) || newcap == mincap))
    return newcap;

  newcap = (newct + (newct >> 3) + 6) & ~3ul;

  if (newct - oldct > newcap - newct)
    newcap = (newct + 3) & ~3ul;

  return newcap;
}

usize pad_array_size(usize newct, usize oldcap, usize mincap, double loadf) {
  usize newcap = MAX(oldcap, mincap);
    if (newct > newcap * loadf) {
    do {
      newcap <<= 1;
    } while (newct > newcap * loadf);
  } else if (newct < (newcap >> 1) * loadf && newcap > mincap) {
    do {
      newcap >>= 1;
    } while (newct < (newcap >> 1) * loadf && newcap > mincap);
  }
  return newcap;
}

usize pad_table_size(usize newct, usize oldcap) {
  return pad_array_size(newct, oldcap, MIN_CAP, TABLE_LOAD);
}

static usize ord_size(usize cap) {
  assert(cap <= MAX_CAP);
  assert(cap >= MIN_CAP);

  if (cap <= INT8_MAX)
    return cap;

  if (cap <= INT16_MAX)
    return cap * sizeof(sint16);

  if (cap <= INT32_MAX)
    return cap * sizeof(sint32);

  return cap * sizeof(sint64);
}

void* allocate_ord(usize cap) {
  usize total = ord_size(cap);
  void* out   = allocate(total);

  memset(out, -1, total);

  return out;
}

void* reallocate_ord(void* ptr, usize newc, usize oldc) {
  usize oldt = ord_size(oldc), newt = ord_size(newc);

  ptr = reallocate(ptr, newt, oldt);

  memset(ptr, -1, newt);

  return ptr;
}

void deallocate_ord(void* ptr, usize cap) {
  deallocate(ptr, ord_size(cap));
}

value_t* allocate_table(usize cap) {
  value_t* out = allocate(cap * 2 * sizeof(value_t));

  for (usize i=0; i<cap*2; i++)
    out[i] = NOTFOUND;

  return out;
}

value_t* reallocate_table(value_t* ptr, usize newc, usize oldc) {
  value_t* out = reallocate(ptr, newc*2*sizeof(value_t), oldc*2*sizeof(value_t));

  for (usize i=oldc*2; i<newc*2; i++)
    out[i] = NOTFOUND;

  return out;
}

void deallocate_table(value_t* ptr, usize cap) {
  deallocate(ptr, cap * 2 * sizeof(value_t));
}

void rehash_table(table_t* table) {
  value_t* entries = table->table;
  usize count      = table->count;
  usize ocap       = table->ocap;

  if (ocap <= INT8_MAX) {
    sint8* ord = table->ord8;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  } else if (ocap <= INT16_MAX) {
    sint16* ord = table->ord16;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  } else if (ocap <= INT32_MAX) {
    sint32* ord = table->ord32;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  } else {
    sint64* ord = table->ord64;

    for (usize i=0; i<count; i++) {
      uhash h = rl_hash(entries[i << 1]);
      usize j = h & (ocap - 1);

      while (ord[j] > -1)
        j = j & (ocap - 1);

      ord[j] = i;
    }
  }
}

void resize_table(table_t* self, usize cnt) {
  usize tcap = pad_alist_size(cnt, self->count, self->tcap, MIN_CAP);

  if (tcap != self->tcap) {
    self->table = reallocate_table(self->table, tcap, self->tcap);
    self->tcap  = tcap;
  }

  usize ocap = pad_table_size(cnt, self->ocap);

  if (ocap != self->ocap) {
    self->ord  = reallocate_ord(self->ord, ocap, self->ocap);
    self->ocap = ocap;
    rehash_table(self);
  }
}

long table_locate(table_t* self, value_t key, usize* buffer) {
  value_t* table = self->table;
  usize    ocap  = self->ocap;
  usize    omask = ocap - 1;
  uhash    khash = rl_hash(key);
  usize    order = khash & omask;
  long     index = -1;

  if (ocap <= INT8_MAX) {
    sint8* ord = self->ord8;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order =  (order + 1) & omask;
    }
  } else if (ocap <= INT16_MAX) {
    sint16* ord = self->ord16;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order = (order + 1) & omask;
    }
  } else if (ocap <= INT32_MAX) {
    sint32* ord = self->ord32;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order = (order + 1) & omask;
    }
  } else {
    sint64* ord = self->ord64;

    while ((index=ord[order]) > -1) {
      if (rl_equal(key, table[index*2]))
        break;

      order = (order + 1) & omask;
    }
  }

  if (buffer)
    *buffer = order;

  return index;
}

// sacred methods -------------------------------------------------------------
void print_table(value_t x, port_t* ios) {
  rl_puts("#table(", ios);

  table_t* tab = as_table(x); print_values(tab->count*2, tab->table, ios, true);

  rl_putc(')', ios);
}

bool equal_tables(void* xp, void* yp) {
  table_t* xt = xp, * yt = yp;

  return equal_values(xt->count*2, xt->table, yt->count*2, yt->table);
}

int compare_tables(value_t x, value_t y) {
  table_t* xt = as_table(x), * yt = as_table(y);

  return compare_values(xt->count*2, xt->table, yt->count*2, yt->table);
}

// lifetime methods -----------------------------------------------------------
int init_table(void* ptr, void* dat) {
  (void)dat;

  table_t* self = ptr;
  self->count   =   0;
  self->tcap    = pad_alist_size(0, 0, 0, 8);
  self->ocap    = pad_table_size(0, 0);
  self->table   = allocate_table(self->tcap);
  self->ord     = allocate_ord(self->ocap);

  return 0;
}

void trace_table(void* ptr) {
  table_t* self = ptr;

  mark_values(self->count*2, self->table);
}

void free_table(void* ptr) {
  table_t* self = ptr;

  deallocate_ord(self->ord, self->ocap);
  deallocate_table(self->table, self->tcap);
}

// misc -----------------------------------------------------------------------
void reset_table(table_t* self) {
  free_table(self);
  init_table(self, NULL);
}

// ctors ----------------------------------------------------------------------
table_t* table(void) {
  object_init_t ini = {
    .type  =&TableType
  };

  table_t* out = allocate(sizeof(table_t));

  init_object(out, &ini);

  return out;
}

// getters & setters ----------------------------------------------------------
value_t table_get(table_t* self, value_t key) {
  long index = table_locate(self, key, NULL);

  if (index == -1)
    return NOTFOUND;

  return self->table[(index << 1) + 1];
}

value_t table_set(table_t* self, value_t key, value_t val) {
  value_t out; usize order;

  resize_table(self, self->count+1); // preemptively, so that hashed index is valid
  
  long index = table_locate(self, key, &order);

  if (index == -1) {
    out = NOTFOUND;

    if (self->ocap <= INT8_MAX)       self->ord8[order]  = self->count;
    else if (self->ocap <= INT16_MAX) self->ord16[order] = self->count;
    else if (self->ocap <= INT32_MAX) self->ord32[order] = self->count;
    else                              self->ord64[order] = self->count;

    self->table[self->count*2]   = key;
    self->table[self->count*2+1] = val;
    self->count++;
  } else {
    out                    = self->table[index*2+1];
    self->table[index*2+1] = val;
  }

  return out;
}

value_t table_del(table_t* self, value_t key) {
  value_t out; usize order;

  long index = table_locate(self, key, &order);

  if (index == -1)
    out = NOTFOUND;

  else {
    if (self->ocap <= INT8_MAX)       self->ord8[order] =  -1;
    else if (self->ocap <= INT16_MAX) self->ord16[order] = -1;
    else if (self->ocap <= INT32_MAX) self->ord32[order] = -1;
    else                              self->ord64[order] = -1;

    self->table[order*2]   = NOTFOUND;
    self->table[order*2+1] = NOTFOUND;

    resize_table(self, self->count-1);
  }

  return out;
}

// alist ----------------------------------------------------------------------
// globals --------------------------------------------------------------------

void print_alist(value_t self, port_t* ios);
bool equal_alists(void* px, void* py);
int  compare_alists(value_t x, value_t y);
int  init_alist(void* ptr, void* dat);
void trace_alist(void* ptr);
void free_alist(void* ptr);

data_type_t AlistType = {
  .type={
    .obj={
      .type  =DATA_TYPE,
      .frozen=true,
      .gray  =true,
    },
    .name="alist",
    .idno=ALIST
  },
  .size   =sizeof(alist_t),
  .print  =print_alist,
  .equal  =equal_alists,
  .compare=compare_alists,
  .init   =init_alist,
  .trace  =trace_alist,
  .free   =free_alist
};

// utilities ------------------------------------------------------------------
void resize_alist(alist_t* self, usize cnt) {
  usize newc = pad_alist_size(cnt, self->count, self->cap, MIN_CAP);

  if (newc != self->cap) {
    self->array = reallocate(self->array, newc * sizeof(value_t), self->cap * sizeof(value_t));
    self->cap   = newc;
  }
}

// sacred methods -------------------------------------------------------------
void print_alist(value_t x, port_t* ios) {
  alist_t* alist = as_alist(x);

  rl_puts("#alist(", ios);
  print_values(alist->count, alist->array, ios, false);
  rl_putc(')', ios);
}

bool equal_alists(void* px, void* py) {
  alist_t* ax = px, * ay = py;

  return equal_values(ax->count, ax->array, ay->count, ay->array);
}

int compare_alists(value_t x, value_t y) {
  alist_t* ax = as_alist(x), * ay = as_alist(y);

  return equal_values(ax->count, ax->array, ay->count, ay->array);
}

// lifetime -------------------------------------------------------------------
int init_alist(void* ptr, void* dat) {
  (void)dat;

  alist_t* ax = ptr;
  ax->count   = 0;
  ax->cap     = pad_alist_size(0, 0, 0, 8);
  ax->array   = allocate(ax->cap * sizeof(value_t));

  return 0;
}

void trace_alist(void* ptr) {
  alist_t* alist = ptr;

  mark_values(alist->count, alist->array);
}

void free_alist(void* ptr) {
  alist_t* self = ptr;

  deallocate(self->array, self->cap * sizeof(value_t));
}

// misc ----------------------------------------------------------------------------
void reset_alist(alist_t* self) {
  free_alist(self);
  init_alist(self, NULL);
}

usize alist_push(alist_t* self, value_t val) {
  usize out = self->count++;
  resize_alist(self, self->count);
  self->array[out] = val;
  return out;
}

value_t alist_pop(alist_t* self) {
  assert(self->count);

  value_t out = self->array[--self->count];
  resize_alist(self, self->count);

  return out;
}
