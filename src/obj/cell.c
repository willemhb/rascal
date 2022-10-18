#include "obj/cell.h"

// implementation
void trace_cons( object_t *obj );
void trace_icons( object_t *obj );
void trace_pair( object_t *obj );

// globals
extern type_t DataTypeOb;

// cons type initialization
layout8_t ConsTypeLayout =
  {
    {
      .base_size = sizeof(cons_t),
      .n_fields  = 3
    },
    
    {
      { 8, NTUNSIGNED, false, 16  },
      { 8, NTUNSIGNED, true,  24  },
      { 8, NTPTR,      false, 32  },
    }
  };

dtype_impl_t ConsImplOb =
  {
    .value       = Ctype_pointer,
    .data_size   = 8,
    .fits_fixnum = false,
    .fits_word   = false,
    .layout      = &ConsTypeLayout.layout,
    .singleton   = NUL_VAL,
    .trace       = trace_cons,
    .free        = NULL
  };

type_t ConsTypeOb =
  {
    .isa=isa_dtype,
    .has=dtype_has,

    .dtype=&ConsImplOb
  };

// icons type implementation
layout8_t IConsTypeLayout =
  {
    {
      .base_size = sizeof(icons_t),
      .n_fields  = 3,
    },

    {
      { 8, NTUNSIGNED, false, 16  },
      { 8, NTUNSIGNED, true,  24  },
      { 8, NTUNSIGNED, true,  32  },
    }
  };

dtype_impl_t IConsImplOb =
  {
    .value      = Ctype_pointer,
    .data_size  =8,
    .fits_fixnum=false,
    .fits_word  =false,
    .layout     =&IConsTypeLayout.layout,
    .singleton  =NUL_VAL,
    .trace      =trace_icons,
    .free       =NULL
  };

