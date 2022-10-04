#ifndef rascal_number_h
#define rascal_number_h

#include "val.h"

#define is_real(val)     ((val)&QNAN != QNAN)
#define is_chr(val)      (((val)&TMASK)==CHRTAG)
#define as_real(val)     (((val_data_t)(val)).as_real)
#define as_chr(val)      ((int32_t)((((val_data_t)(val)).as_val)&(val_t)UINT32_MAX))


// forward declarations for object apis
void      prin_real(port_t *port, val_t val);
void      prin_chr(port_t *port, val_t val);

#endif
