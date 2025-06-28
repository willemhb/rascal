
/* Implementation for chunk type. */

// headers --------------------------------------------------------------------
#include <stdio.h>

#include "sys/opcode.h"

#include "data/array.h"

#include "data/types/chunk.h"
#include "data/types/buf16.h"

// macros ---------------------------------------------------------------------

// C types --------------------------------------------------------------------

// function prototypes --------------------------------------------------------
void trace_chunk(void* ptr);

// globals --------------------------------------------------------------------

// function definitions -------------------------------------------------------
// internal -------------------------------------------------------------------
void trace_chunk(void* ptr) {
  Chunk* chunk = ptr;

  mark_obj(chunk->vars);
  mark_obj(chunk->vals);
  mark_obj(chunk->code);
}

// external -------------------------------------------------------------------
Chunk* mk_chunk(Env* vars, Alist* vals, Buf16* code) {
  Chunk* out = mk_obj(EXP_CHUNK, 0);

  out->vars = vars;
  out->vals = vals;
  out->code = code;

  return out;
}

void dis_chunk(Chunk* chunk) {
  instr_t* instr = chunk->code->binary.vals;
  int offset     = 0, max_offset = chunk->code->binary.count;

  printf("%-8s %-16s %-5s %-5s\n\n", "line", "instruction", "input", "input");

  while ( offset < max_offset ) {
    OpCode op  = instr[offset];
    int argc   = op_arity(op);
    char* name = op_name(op);

    switch ( argc ) {
   
      case 1: { 
        instr_t arg = instr[offset+1];
        printf("%.8d %-16s %.5d -----\n", offset, name, arg);
        offset += 2;                                           // advance past argument
        break;
      }

      case -2: { // variadic
        int arg = instr[offset+1];
        printf("%.8d %-16s %.5d -----\n", offset, name, arg);
        offset++;

        for ( int i=0; i < arg; i++, offset += 2 ) {
          int x = instr[offset+1], y = instr[offset+2];
          printf("%.8d ---------------- %.5d %.5d\n", offset, x, y);
        }

        break;
      }

      default:
        printf("%.8d %-16s ----- -----\n", offset, name);
        offset++;
        break;
    }
  }
}

// initialization -------------------------------------------------------------
void toplevel_init_data_type_chunk(void) {
  Types[EXP_CHUNK] = (ExpTypeInfo){
    .type     = EXP_CHUNK,
    .name     = "chunk",
    .obsize   = sizeof(Chunk),
    .trace_fn = trace_chunk
  };
}
