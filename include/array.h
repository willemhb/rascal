#ifndef rascal_array_h
#define rascal_array_h

#include "rascal.h"

// globals --------------------------------------------------------------------
extern value_t r_kw_s8, r_kw_u8, r_kw_s16, r_kw_u16, r_kw_s32, r_kw_u32, r_kw_s64, r_kw_f64;

// constructors ---------------------------------------------------------------
value_t vector( size_t n, value_t *args );
index_t vector_s( size_t n, value_t *args );

value_t  binary( size_t n, Ctype_t ctype, void *args );
index_t  binary_s( size_t n, Ctype_t ctype, void *args );
value_t  resize_binary( value_t v, size_t n );

value_t  string( char *chars );
index_t  string_s( size_t n, char *chars );
value_t  resize_string( value_t s, size_t n );

// accessors & utilities ------------------------------------------------------
long validate_array_index( const char *fname, long n, value_t array );

value_t resize_vector( value_t vec, size_t n );
value_t vector_get( value_t vec, size_t n );
value_t vector_set( value_t vec, size_t n, value_t x );
value_t vector_put( value_t vec, value_t x );
value_t vector_get_s( const char *fname, value_t *vec, long n );
value_t vector_set_s( const char *fname, value_t *vec, long n, value_t x);
value_t vector_put_s( const char *fname, value_t *vec, value_t x);

Ctype_t  get_Ctype( value_t x );
value_t  resize_binary( value_t bin, size_t n );
fixnum_t binary_get( value_t bin, size_t n );
fixnum_t binary_set( value_t bin, size_t n, fixnum_t x );
value_t  binary_put( value_t bin, fixnum_t x );
value_t  binary_get_s( const char *fname, value_t *bin, long n );
value_t  binary_set_s( const char *fname, value_t *bin, long n, value_t f );
value_t  binary_put_s( const char *fname, value_t *bin, value_t f);

value_t  resize_string( value_t str, size_t n );
char     string_get( value_t str, size_t n );
char     string_set( value_t str, size_t n, char x );
value_t  string_put( value_t str, char x );
value_t  string_get_s( const char *fname, value_t *bin, long n );
value_t  string_set_s( const char *fname, value_t *bin, long n, value_t f );
value_t  string_put_s( const char *fname, value_t *bin, value_t f);

// predicates -----------------------------------------------------------------
bool is_vector( value_t x );
bool is_binary( value_t x );
bool is_string( value_t x );

// safecasts ------------------------------------------------------------------
vector_t *tovector( const char *fname, value_t x );
binary_t *tobinary( const char *fname, value_t x );
string_t *tostring( const char *fname, value_t x );

// methods -------------------------------------------------------------------
size_t vector_prin( FILE *ios, value_t x );
size_t binary_prin( FILE *ios, value_t x );
size_t string_prin( FILE *ios, value_t x );

int    vector_order( value_t x, value_t y );
int    binary_order( value_t x, value_t y );
int    string_order( value_t x, value_t y );

size_t vector_sizeof( value_t x );
size_t binary_sizeof( value_t x );
size_t string_sizeof( value_t x );

// builtins -------------------------------------------------------------------
void r_builtin(vector);
void r_builtin(binary);
void r_builtin(string);

void r_builtin(is_vector);
void r_builtin(is_binary);
void r_builtin(is_string);
void r_builtin(is_empty);

void r_builtin(len);
void r_builtin(nth);
void r_builtin(xth);
void r_builtin(put);

// initialization -------------------------------------------------------------
void array_init( void );

// utility macros -------------------------------------------------------------
#define asvector(x)  ((vector_t*)pval(x))
#define asbinary(x)  ((binary_t*)pval(x))
#define asstring(x)  ((string_t*)pval(x))

#define asize(x)   (((size_t*)pval(x))[1])
#define adata(x)   (((void**)pval(x))[2])
#define alength(x) (((size_t*)pval(x))[3])
#define s8data(x)  (((char**)pval(x))[2])
#define u8data(x)  (((uchar**)pval(x))[2])
#define s16data(x) (((short**)pval(x))[2])
#define u16data(x) (((ushort**)pval(x))[2])
#define s32data(x) (((int**)pval(x))[2])
#define u32data(x) (((uint**)pval(x))[2])
#define s64data(x) (((long**)pval(x))[2])
#define f64data(x) (((double**)pval(x))[2])

#define init_array(o, t, C, e, l, s)			\
  do {							\
    init_ob( o, t, C, e, true, sizeof(vector_t));	\
    asize( o )   = s;					\
    adata( o )   = (uchar*)o + sizeof(vector_t);	\
    alength( o ) = l;					\
  } while (0)

#define init_vector(o, l, s)			\
  init_array(o, tag_vector, C_sint64, 0, l, s)

#define init_binary(o, C, l, s)			\
  init_array(o, tag_binary, C, 0, l, s)

#define init_string(o, l, s)				\
  init_array(o, tag_string, C_sint8, enc_ascii, l, s+1)


#endif
