// system includes
#include <stdio.h>
#include <stdlib.h>

// local includes
#include "common.h"
#include "utils.h"

// typedefs
typedef uintptr_t val_t;
typedef double    real_t;
typedef int64_t   int_t;

typedef union
{
  real_t as_real;
  val_t  as_val;

  struct
  {
    int_t        :  4;
    int_t as_int : 44;
    int_t        : 16;
  };
  
} val_data_t;

// tags
#define QNAN   0x7ffc000000000000ul
#define SIGN   0x8000000000000000ul

#define NUMBER 0x7ffc000000000000ul
#define REAL   (NUMBER|0ul)
#define INT    (NUMBER|1ul)

// bit manipulation
#define as_real(val) (((val_data_t)(val)).as_real)
#define as_int(val)  ((int_t)(((val_data_t)(val)).as_int))
#define as_val(val)  (((val_data_t)(val)).as_val)

// memory management
#define safe_alloc(allcfun, ...)		\
  ({						\
    void *out = allcfun( __VA_ARGS__ );		\
    if (out == NULL)				\
      {						\
	printf("error at %s:%d:%s: OOM.",	\
	       __FILE__,			\
	       __LINE__,			\
	       __func__);			\
	exit(1);				\
      }						\
    out;					\
  })

// reader
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct rentry_t
{
  int32_t  dispatch;
  uint32_t hash;
  val_t (*handler)(FILE *stream, char disp);
} rentry_t;

rentry_t *new_rentry(void)
{
  return safe_alloc( malloc, sizeof(rentry_t) );
}

void free_rentry(rentry_t *re)
{
  free(re);
}

void init_rentry(rentry_t *re, char dispatch, val_t (*handler)(FILE *stream, char disp))
{
  re->dispatch = dispatch;
  re->handler  = handler;
}

typedef struct readt_t
{
  rentry_t **data;
  int        len;
  int        cap;
} readt_t;

// main
int main(const int argc, const char *argv[argc])
{
  (void)argv;

  return 0;
}
