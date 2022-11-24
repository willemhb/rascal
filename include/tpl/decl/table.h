#ifndef rascal_tpl_table_h
#define rascal_tpl_table_h

// convenience macros
#define TABLE( E )				\
  OBJHEAD;					\
  size_t   count;				\
  size_t   capacity;				\
  E      **entries

#endif
