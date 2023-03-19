#include "hamt.h"


// arr node -------------------------------------------------------------------
#define ARR_MAXH 8
#define ARR_MAXN 64

arr_node_t* unfreeze_arr_node(arr_node_t* node);

usize pad_arr_node_size(usize n, usize oldc) {
  oldc = MAX(1u, oldc);

  if (n > oldc || n < (oldc >> 1))
    return ceil2(n);

  return oldc;
}

void resize_arr_node(arr_node_t* node, usize n) {
  assert(n <= ARR_MAXN);

  usize padded = pad_arr_node_size(n, node->cap);

  if (padded != node->cap) {
    node->values = reallocate(node->values, node->cap * sizeof(value_t), padded * sizeof(value_t));
    node->cap    = padded;
  }

  node->len = n;
}

void init_arr_node(arr_node_t* self, uint16 len, uint32 height, void* src) {
  init_object(self, ARR_NODE, 0);

  self->len    = len;
  self->cap    = pad_arr_node_size(len, 0);
  self->height = height;
  self->values = allocate(self->cap * sizeof(value_t));

  if (src)
    memcpy(self->values, src, self->len * sizeof(value_t));
}

arr_node_t* arr_node(uint16 len, uint32 height, void* src) {
  arr_node_t* out = allocate(sizeof(arr_node_t));

  init_arr_node(out, len, height, src);

  return out;
}

arr_node_t* unfreeze_arr_node(arr_node_t* node) {
  if (has_flag(node, FROZEN))
    node = arr_node(node->len, node->height, node->values);

  return node;
}

void freeze_arr_node(arr_node_t* node) {
  if (has_flag(node, FROZEN))
    return;

  set_flag(node, FROZEN);

  if (node->height)
    for (int i=0; i<node->len; i++)
      freeze_arr_node(node->children[i]);
}

value_t arr_node_get(arr_node_t* node, usize n) {
  while (node->height)
      node = node->children[n >> (node->height * 6) & 0x3f];

  return node->values[n & 0x3f];
}

arr_node_t* arr_node_set(arr_node_t* node, usize n, value_t v) {
  bool frozen = has_flag(node, FROZEN);
  arr_node_t* buffer[ARR_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[ARR_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h;

  for (;;h=node->height) {
    int i = n >> (node->height * 6) & 0x3f;
    indices[h] = i;
    buffer[h] = unfreeze_arr_node(node);
    
    if (h)
      node = node->children[i];

    else
      break;
  }

  for (h=0; h <= maxh; h++) {
    int i = indices[h];
    
    if (h)
      buffer[h]->children[i] = buffer[h-1];

    else
      buffer[h]->values[i] = v;
  }

  if (frozen)
    for (h=0; h <= maxh; h++)
      freeze(buffer[h]);

  return buffer[maxh];
}

arr_node_t* arr_node_append(arr_node_t* node, value_t v) {
  bool frozen = has_flag(node, FROZEN);
  arr_node_t* buffer[ARR_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[ARR_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h;

  for (;;h=node->height) {
    int i = node->len-1;
    indices[h] = i;
    buffer[h]  = node;

    if (h)
      node = node->children[i];

    else
      break;
}

  for (h=0; h<=maxh; h++) {
    if (indices[h] < 63) {
      indices[h]++;
      buffer[h] = unfreeze_arr_node(buffer[h]);
      resize_arr_node(buffer[h], indices[h]+1);

      for (h=h+1;h<=maxh; h++)
        buffer[h] = unfreeze_arr_node(buffer[h]);
    } else {
      arr_node_t* tmp = buffer[h], * new = arr_node(1, h, NULL);

      buffer[h]  = new;
      indices[h] = 0;

      if (h == maxh) {
        buffer[h+1]  = arr_node(1, h+1, &tmp);
        indices[h+1] = 0;
        maxh++;
      }
    }
  }

  for (h=0; h<=maxh; h++) {
    int i = indices[h];

    if (h)
      buffer[h]->children[i] = buffer[h-1];

    else
      buffer[h]->values[i] = v;

    if (frozen)
      freeze(buffer[h]);
  }

  return buffer[maxh];
}

arr_node_t* arr_node_pop(arr_node_t* node) {
  bool frozen = has_flag(node, FROZEN);
  arr_node_t* buffer[ARR_MAXH] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  int indices[ARR_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 }, h = node->height, maxh = h, minh = 0;

  for (;;h=node->height) {
    int i = node->len-1;
    indices[h] = i;
    buffer[h]  = node;

    if (h)
      node = node->children[i];

    else
      break;
  }

  for (h=0; h<=maxh; h++) {
    int i = indices[h];

    if (i > 0) {
      if (h == maxh && i == 1) // special case: the root only has one nonempty child, which becomes the new tree
        return buffer[h]->children[0];

      minh = h;
      buffer[h] = unfreeze_arr_node(buffer[h]);
      resize_arr_node(buffer[h], i);

      for (h=h+1;h<=maxh; h++)
        buffer[h] = unfreeze_arr_node(buffer[h]);
    }
  }

  for (h=minh+1; h<=maxh; h++) {
    int i = indices[h];

    buffer[h]->children[i] = buffer[h-1];

    if (frozen)
      freeze(buffer[h]);
  }

  return buffer[maxh];
}

// vector ---------------------------------------------------------------------
struct vector_t EmptyVector = {
  .obj ={
    .next =NULL,
    .hash =0,
    .flags=FROZEN,
    .type =VECTOR,
    .gray =false,
    .black=true
  },
  .len =0,
  .root=NULL
};

void init_vector(vector_t* self, usize n, arr_node_t* root) {
  init_object(self, VECTOR, FROZEN);
  self->len  = n;
  self->root = root;
}
vector_t* mk_vector(usize n, arr_node_t* root) {
  if (n == 0)
    return &EmptyVector;

  vector_t* out = allocate(sizeof(vector_t));

  init_vector(out, n, root);

  return out;
}

value_t vector(usize n, value_t* vals) {
  vector_t* out;
  
  if (n == 0)
    out = &EmptyVector;

  else {
    arr_node_t* root = arr_node(0, 0, NULL);

    for (usize i=0; i<n; i++)
      root = arr_node_append(root, vals[i]);

    out = mk_vector(n, root);
  }

  return object(out);
}

value_t vector_get(vector_t* self, usize n) {
  assert(n < self->len);

  return arr_node_get(self->root, n);
}

vector_t* vector_set(vector_t* self, usize n, value_t v) {
  assert(n < self->len);

  arr_node_t* root = arr_node_set(self->root, n, v);

  return mk_vector(self->len, root);
}

vector_t* vector_add(vector_t* self, value_t v) {
  assert(self->len < FIXNUM_MAX);

  arr_node_t* root = arr_node_append(self->root, v);

  return mk_vector(self->len, root);
}

vector_t* vector_pop(vector_t* self) {
  assert(self->len);
  
  if (self->len == 1)
    return &EmptyVector;

  arr_node_t* root = arr_node_pop(self->root);

  return mk_vector(self->len-1, root);
}

// map node -------------------------------------------------------------------
#define MAP_MAXH 8
#define MAP_MAXN 64

map_node_t* unfreeze_map_node(map_node_t* self);
void        resize_map_node(map_node_t* self, usize n);

usize pad_map_node_size(usize n, usize oldc) {
  oldc = MAX(1u, oldc);

  if (n > oldc || n < (oldc >> 1))
    return ceil2(n);

  return oldc;
}

uhash get_hash_key(int h, uhash c) {
  return c >> (h * 6) & 0x3f;
}

uhash get_hash_mask(int h, uhash c) {
  return 1ul << get_hash_key(h, c);
}

int get_hash_index(int h, usize b, uhash c) {
  uhash m = get_hash_mask(c, h);

  if (b & m)
    return popcnt(b & (m-1));

  return -1;
}

bool  compare_hashes(uhash hx, uhash hy, int h) {
  if (h > -1) // if h is -1, compare the full hashes --------------------------
    hx = get_hash_key(h, hx), hy = get_hash_key(h, hy);

  return hx == hy;
}

uhash map_node_key(map_node_t* node, uhash h) {
  return get_hash_key(node->height, h);
}

usize map_node_mask(map_node_t* node, uhash h) {
  return get_hash_mask(node->height, h);
}

int map_node_index(map_node_t* node, uhash h) {
  return get_hash_index(node->height, node->bitmap, h);
}

int add_key_to_map_node(map_node_t* node, uhash h) {
  usize mask = map_node_mask(node, h);

  if (node->bitmap & mask)
    return popcnt(node->bitmap & (mask - 1));

  node->bitmap |= mask;
  int i         = popcnt(node->bitmap & (mask - 1));

  resize_map_node(node, node->len+1);

  if (i < node->len)
    memmove(node->entries+i+1, node->entries+i, (node->len - i) * sizeof(void*));

  node->entries[i] = NULL;

  return i;
}

void resize_map_node(map_node_t* node, usize n) {
  assert(n <= MAP_MAXN);

  usize padded = pad_map_node_size(n, node->cap);

  if (padded != node->cap) {
    node->entries = reallocate(node->entries,
                               node->cap * sizeof(value_t),
                               padded * sizeof(value_t));
    node->cap     = padded;
  }

  node->len = n;
}


void init_map_node(map_node_t* self, usize bitmap, uint16 len, uint32 height, void* src) {
  init_object(self, MAP_NODE, 0);

  if (len == 0)
    len = popcnt(bitmap);

  self->len     = len;
  self->cap     = pad_map_node_size(len, 0);
  self->height  = height;
  self->entries = allocate(self->cap * sizeof(void*));
  self->bitmap  = bitmap;

  if (src)
    memcpy(self->entries, src, self->len * sizeof(void*));
}

map_node_t* map_node(usize bitmap, uint16 len, uint32 height, void* src) {
  map_node_t* out = allocate(sizeof(map_node_t));

  init_map_node(out, bitmap, len, height, src);

  return out;
}

map_node_t* unfreeze_map_node(map_node_t* node) {
  if (has_flag(node, FROZEN))
    node = map_node(node->bitmap, node->len, node->height, node->entries);

  return node;
}

void freeze_map_node(map_node_t* node) {
  if (has_flag(node, FROZEN))
    return;

  set_flag(node, FROZEN);

  if (node->height)
    for (int i=0; i<node->len; i++)
      if (node->entries[i]->type == MAP_NODE)
        freeze_map_node((map_node_t*)node->entries[i]);
}

tuple_t* map_node_get(map_node_t* node, value_t k) {
  uhash h = hash(k); tuple_t* out = NULL;
  
  for (;;) {
    int i = map_node_index(node, h);

    if (i == -1)
      break;

    object_t* o = node->entries[i];
    type_t t    = o->type;

    if (t == MAP_NODE)
      node = (map_node_t*)o;

    else if (t == TUPLE) {
      tuple_t* e = (tuple_t*)o;
      
      if (equal(k, first(e)))
        out = e;

      break;
    }

    else {
      assert(t == LIST);
      list_t* l = (list_t*)o;

      while (!out && l->len) {
        tuple_t* e = as_ptr(l->head);

        if (equal(k, first(e)))
          out = e;

        else
          l = l->tail;
      }

      break;
    }

  }

  return out;
}

bool map_node_set(map_node_t** root, value_t k, value_t v) {
  map_node_t* node              = *root;
  map_node_t* buffer[MAP_MAXH]  = {  0,  0,  0,  0,  0,  0,  0,  0 };
  int         indices[MAP_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
  bool        frozen            = has_flag(node, FROZEN), out = true;
  uhash       kh                = hash(k);
  int         height            = node->height, maxh = height, minh = height, i;
  object_t*   entry             = NULL;

  // find path to entry node --------------------------------------------------
  for (;;height=node->height, minh=height) {
    buffer[height]  = node;
    indices[height] = i = map_node_index(node, kh);

    if (i == -1) {
      buffer[height]  = unfreeze_map_node(buffer[height]);
      indices[height] = i = add_key_to_map_node(node, kh);
      break;
    }

    object_t* o = node->entries[i];

    if (o->type == MAP_NODE)
      node = (map_node_t*)o;

    else {
      entry = o;
      break;
    }
  }

  // create entry node and resolve collisions ---------------------------------
  if (entry == NULL)
  // no entry exists for this key (just create a new pair) --------------------
    entry = as_object(pair(k, v));

  else if (entry->type == TUPLE) {
    // one entry exists -------------------------------------------------------
    tuple_t* t = (tuple_t*)entry;
    value_t ck = first(t);

    if (equal(ck, k)) {
      // same key, replace with new value -------------------------------------
      out   = false;
      entry = as_object(pair(k, v));

    } else {
      // minor collision (equal subhashes, find where they diverge) -----------
      value_t ch = hash(ck);

      for (height=minh-1; height >= 0; minh=height, height--) {
        uhash km = get_hash_mask(height, kh);
        uhash cm = get_hash_mask(height, ch);
        buffer[height] = map_node(cm, 0, minh, &entry);
        indices[height] = add_key_to_map_node(buffer[minh], kh);

        if (km != cm)
          break;
      }

      if (height == -1) {
        // major collision (create new collision node) ------------------------
        value_t vals[2] = { pair(k, v), object(entry) };
        entry = as_object(list(2, vals));
      }
    }
  } else {
    // major collision (equal hashes, add to existing collision node) ---------
    assert(entry->type == LIST);
    list_t* entries = (list_t*)entry;
    entry = as_object(cons(pair(k, v), entries));

    while (entries->head) {
      tuple_t* t = as_tuple(entries->head);

      if (equal(first(t), k)) {
        out = false;
        break;
      }

      entries = entries->tail;
    }
  }
  
  // walk from minh up to maxh, attaching new nodes ---------------------------
  for (height=minh; height<=maxh; height++) {    
    buffer[height] = unfreeze_map_node(buffer[height]);
    
    if (height==minh)
      buffer[height]->entries[indices[height]] = entry;

    else
      buffer[height]->entries[indices[height]] = (object_t*)buffer[indices[height-1]];

    if (frozen)
      freeze(buffer[height]);
  }

  *root = buffer[maxh];

  return out;
}

bool map_node_put(map_node_t** root, value_t k, value_t v) {
  map_node_t* node              = *root;
  map_node_t* buffer[MAP_MAXH]  = {  0,  0,  0,  0,  0,  0,  0,  0 };
  int         indices[MAP_MAXH] = { -1, -1, -1, -1, -1, -1, -1, -1 };
  bool        frozen            = has_flag(node, FROZEN);
  uhash       kh                = hash(k);
  int         height            = node->height, maxh = height, minh = height, i;
  object_t*   entry             = NULL;

  // find path to entry node --------------------------------------------------
  for (;;height=node->height, minh=height) {
    buffer[height]  = node;
    indices[height] = i = map_node_index(node, kh);

    if (i == -1) {
      buffer[height]  = unfreeze_map_node(buffer[height]);
      indices[height] = i = add_key_to_map_node(node, kh);
      break;
    }

    object_t* o = node->entries[i];

    if (o->type == MAP_NODE)
      node = (map_node_t*)o;

    else {
      entry = o;
      break;
    }
  }

  // create entry node and resolve collisions ---------------------------------
  if (entry == NULL)
  // no entry exists for this key (just create a new pair) --------------------
    entry = as_object(pair(k, v));

  else if (entry->type == TUPLE) {
    // one entry exists -------------------------------------------------------
    tuple_t* t = (tuple_t*)entry;
    value_t ck = first(t);

    if (equal(ck, k))
      // same key, do nothing -------------------------------------------------
      return false;

    else {
      // minor collision (equal subhashes, find where they diverge) -----------
      value_t ch = hash(ck);

      for (height=minh-1; height >= 0; minh=height, height--) {
        uhash km = get_hash_mask(height, kh);
        uhash cm = get_hash_mask(height, ch);
        buffer[height] = map_node(cm, 0, minh, &entry);
        indices[height] = add_key_to_map_node(buffer[minh], kh);

        if (km != cm)
          break;
      }

      if (height == -1) {
        // major collision (create new collision node) ------------------------
        value_t vals[2] = { pair(k, v), object(entry) };
        entry = as_object(list(2, vals));
      }
    }
  } else {
    // major collision (equal hashes, add to existing collision node) ---------
    assert(entry->type == LIST);
    list_t* entries = (list_t*)entry;
    entry = as_object(cons(pair(k, v), entries));

    while (entries->head) {
      tuple_t* t = as_tuple(entries->head);

      if (equal(first(t), k))
        return false;

      entries = entries->tail;
    }
  }
  
  // walk from minh up to maxh, attaching new nodes ---------------------------
  for (height=minh; height<=maxh; height++) {
    buffer[height] = unfreeze_map_node(buffer[height]);
    
    if (height==minh)
      buffer[height]->entries[indices[height]] = entry;

    else
      buffer[height]->entries[indices[height]] = (object_t*)buffer[indices[height-1]];

    if (frozen)
      freeze(buffer[height]);
  }

  *root = buffer[maxh];

  return true;
}

bool map_node_del(map_node_t** root, value_t k);

// dict -----------------------------------------------------------------------
struct dict_t EmptyDict = {
  .obj ={
    .next =NULL,
    .hash =0,
    .flags=FROZEN,
    .type =DICT,
    .gray =false,
    .black=true
  },
  .len =0,
  .root=NULL
};

void init_dict(dict_t* self, usize n, map_node_t* root) {
  init_object(self, DICT, FROZEN);

  self->len = n;
  self->root= root;
}

dict_t* mk_dict(usize n, map_node_t* root) {
  dict_t* out = allocate(sizeof(dict_t));

  init_dict(out, n, root);

  return out;
}

value_t dict(usize n, value_t* values) {
  assert((n&1) == 0);
  
  dict_t* out;
  
  if (n == 0)
    out = &EmptyDict;

  else {
    map_node_t* root = map_node(0, 0, 7, NULL);
    usize len = 0;

    for (usize i=0; i<n; i+= 2)
      len += map_node_set(&root, values[i], values[i+1]);

    freeze_map_node(root);

    out = mk_dict(len, root);
  }

  return object(out);
}

value_t dict_get(dict_t* ks, value_t k) {
  tuple_t* kv = map_node_get(ks->root, k);

  if (kv == NULL)
    return NOTFOUND;

  return second(kv);
}

dict_t* dict_set(dict_t* ks, value_t k, value_t v) {
  map_node_t* root = ks->root;
  usize len = ks->len;

  len += map_node_set(&root, k, v);

  return mk_dict(len, root);
}

dict_t* dict_del(dict_t* ks, value_t k) {
  map_node_t* root = ks->root;
  usize len = ks->len;
  len -= map_node_del(&root, k);

  return mk_dict(len, root);
}

// set ------------------------------------------------------------------------
struct set_t EmptySet = {
  .obj ={
    .next =NULL,
    .hash =0,
    .flags=FROZEN,
    .type =SET,
    .gray =false,
    .black=true
  },
  .len =0,
  .root=NULL
};


void init_set(set_t* self, usize n, map_node_t* root) {
  init_object(self, SET, FROZEN);

  self->len = n;
  self->root= root;
}

set_t* mk_set(usize n, map_node_t* root) {
  set_t* out = allocate(sizeof(set_t));

  init_set(out, n, root);

  return out;
}

value_t set(usize n, value_t* values) {
  assert((n&1) == 0);

  set_t* out;

  if (n == 0)
    out = &EmptySet;

  else {
    map_node_t* root = map_node(0, 0, 7, NULL);
    usize len = 0;

    for (usize i=0; i<n; i++)
      len += map_node_set(&root, values[i], values[i]);

    freeze_map_node(root);

    out = mk_set(len, root);
  }

  return object(out);
}

bool set_has(set_t* ks, value_t k) {
  return !!map_node_get(ks->root, k);
}

set_t* set_add(set_t* ks, value_t k) {
  map_node_t* root = ks->root;
  usize len = ks->len;

  len += map_node_put(&root, k, k);

  return mk_set(len, root);
}

set_t* set_del(set_t* ks, value_t k) {
  map_node_t* root = ks->root;
  usize len = ks->len;
  len -= map_node_del(&root, k);

  return mk_set(len, root);
}
