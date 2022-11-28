#include "util/buffer.h"
#include "util/tpl/impl/buffer.h"

/* commentary */

/* C types */

/* globals */

/* API */
MAKE_BUFFER(ascii_buffer, ascii_t);
RESIZE_BUFFER(ascii_buffer, ascii_t);
FREE_BUFFER(ascii_buffer, ascii_t);
BUFFER_PUSH(ascii_buffer, ascii_t);
BUFFER_POP(ascii_buffer, ascii_t);
BUFFER_PUSHN(ascii_buffer, ascii_t, uint);
BUFFER_POPN(ascii_buffer, ascii_t);
WRITETO_BUFFER(ascii_buffer, ascii_t);
READFROM_BUFFER(ascii_buffer, ascii_t);
BUFFER_REF(ascii_buffer, ascii_t);
BUFFER_SET(ascii_buffer, ascii_t);
BUFFER_SWAP(ascii_buffer, ascii_t);

MAKE_BUFFER(latin1_buffer, latin1_t);
RESIZE_BUFFER(latin1_buffer, latin1_t);
FREE_BUFFER(latin1_buffer, latin1_t);
BUFFER_PUSH(latin1_buffer, latin1_t);
BUFFER_POP(latin1_buffer, latin1_t);
BUFFER_PUSHN(latin1_buffer, latin1_t, uint);
BUFFER_POPN(latin1_buffer, latin1_t);
WRITETO_BUFFER(latin1_buffer, latin1_t);
READFROM_BUFFER(latin1_buffer, latin1_t);
BUFFER_REF(latin1_buffer, latin1_t);
BUFFER_SET(latin1_buffer, latin1_t);
BUFFER_SWAP(latin1_buffer, latin1_t);

MAKE_BUFFER(u8_buffer, utf8_t);
RESIZE_BUFFER(u8_buffer, utf8_t);
FREE_BUFFER(u8_buffer, utf8_t);
BUFFER_PUSH(u8_buffer, utf8_t);
BUFFER_POP(u8_buffer, utf8_t);
BUFFER_PUSHN(u8_buffer, utf8_t, uint);
BUFFER_POPN(u8_buffer, utf8_t);
WRITETO_BUFFER(u8_buffer, utf8_t);
READFROM_BUFFER(u8_buffer, utf8_t);
BUFFER_REF(u8_buffer, utf8_t);
BUFFER_SET(u8_buffer, utf8_t);
BUFFER_SWAP(u8_buffer, utf8_t);

MAKE_BUFFER(u16_buffer, utf16_t);
RESIZE_BUFFER(u16_buffer, utf16_t);
FREE_BUFFER(u16_buffer, utf16_t);
BUFFER_PUSH(u16_buffer, utf16_t);
BUFFER_POP(u16_buffer, utf16_t);
BUFFER_PUSHN(u16_buffer, utf16_t, uint);
BUFFER_POPN(u16_buffer, utf16_t);
WRITETO_BUFFER(u16_buffer, utf16_t);
READFROM_BUFFER(u16_buffer, utf16_t);
BUFFER_REF(u16_buffer, utf16_t);
BUFFER_SET(u16_buffer, utf16_t);
BUFFER_SWAP(u16_buffer, utf16_t);

MAKE_BUFFER(u32_buffer, utf32_t);
RESIZE_BUFFER(u32_buffer, utf32_t);
FREE_BUFFER(u32_buffer, utf32_t);
BUFFER_PUSH(u32_buffer, utf32_t);
BUFFER_POP(u32_buffer, utf32_t);
BUFFER_PUSHN(u32_buffer, utf32_t, uint);
BUFFER_POPN(u32_buffer, utf32_t);
WRITETO_BUFFER(u32_buffer, utf32_t);
READFROM_BUFFER(u32_buffer, utf32_t);
BUFFER_REF(u32_buffer, utf32_t);
BUFFER_SET(u32_buffer, utf32_t);
BUFFER_SWAP(u32_buffer, utf32_t);

/* runtime */

/* convenience */
