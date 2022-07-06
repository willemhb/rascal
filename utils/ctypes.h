#ifndef ctypes_h
#define ctypes_h

#include <stddef.h>
#include <stdbool.h>

// definitions ----------------------------------------------------------------
#define C_signed       0x08
#define C_float        0x10
#define C_imaginary    0x20
#define C_complex      0x30
#define C_character    0x40
#define C_pointer      0x80

#define C_8_bit        0x00
#define C_16_bit       0x01
#define C_32_bit       0x02
#define C_64_bit       0x03
#define C_128_bit      0x04
#define C_256_bit      0x05
#define C_512_bit      0x06
#define C_1024_bit     0x07

#define C_size_mask    0x07
#define C_sign_mask    0x08
#define C_qual_mask    0xf0
#define C_pointer_mask 0x40

typedef enum Ctype_t {
  // true unsigned integers
  C_u8  =C_8_bit,
  C_u16 =C_16_bit,
  C_u32 =C_32_bit,
  C_u64 =C_64_bit,

  // true signed integers
  C_s8  =C_signed|C_8_bit,
  C_s16 =C_signed|C_16_bit,
  C_s32 =C_signed|C_32_bit,
  C_s64 =C_signed|C_64_bit,

  // floating point numbers
  C_f32 =C_float|C_signed|C_32_bit,
  C_f64 =C_float|C_signed|C_64_bit,
  C_f128=C_float|C_signed|C_128_bit,

  // imaginary numbers
  C_i32 =C_imaginary|C_signed|C_32_bit,
  C_i64 =C_imaginary|C_signed|C_64_bit,
  C_i128=C_imaginary|C_signed|C_128_bit,

  // complex numbers
  C_c32 =C_complex|C_signed|C_32_bit,
  C_c64 =C_complex|C_signed|C_64_bit,
  C_c128=C_complex|C_signed|C_128_bit,

  // characters (qualified signed integers)
  C_g8  =C_character|C_signed|C_8_bit,
  C_g16 =C_character|C_signed|C_16_bit,
  C_g32 =C_character|C_signed|C_32_bit,

  // pointers (any type)
  C_ptr =C_pointer|C_64_bit,

  // void type
  C_void=C_complex|C_character|C_pointer
} Ctype_t;

// exports --------------------------------------------------------------------
size_t Ctype_size(Ctype_t xct);

bool   Ctype_signedp(Ctype_t xct);
bool   Ctype_intp(Ctype_t xct);
bool   Ctype_uintp(Ctype_t xct);
bool   Ctype_sintp(Ctype_t xct);
bool   Ctype_floatp(Ctype_t xct);
bool   Ctype_imagp(Ctype_t xct);
bool   Ctype_cplxp(Ctype_t xct);
bool   Ctype_charp(Ctype_t xct);
bool   Ctype_ptrp(Ctype_t xct);
bool   Ctype_voidp(Ctype_t xct);

Ctype_t Ctype_common( Ctype_t xct, Ctype_t yct );

#endif
