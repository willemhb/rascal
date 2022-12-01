#ifndef rl_tpl_decl_record_h
#define rl_tpl_decl_record_h

#include "tpl/type.h"

#define GET( T, F, X )     X get_##T##_##F( TYPE(T) *T )
#define SET( T, F, X )     X set_##T##_##F( TYPE(T) *T, X F )
#define IGET( T, M, F, X ) X get_##T##_##F( TYPE(T) *T )
#define ISET( T, M, F, X ) X set_##T##_##F( TYPE(T) *T, X F )
#define PGET( T, P, F, X ) X get_##T##_##P##_##F( TYPE(T) *T, X F )
#define PSET( T, P, F, X ) X set_##T##_##P##_##F( TYPE(T) *T, X F )

#endif
