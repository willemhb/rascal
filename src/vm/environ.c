#include "vm/environ.h"

#include "val/table.h"
#include "val/environ.h"

/* Globals */
Env Globals = {
  .type     = &EnvType,
  .trace    = true,
  .sweep    = false,
  .free     = true,
  .gray     = true,
  
  .scope    = NAMESPACE_SCOPE,
  .bound    = true,
  .toplevel = true,
  
  .index    = -1,
  ._sname   = "global",
  .parent   = NULL,
  .ns       = NULL,
  .locals   = NULL,
  .captured = NULL,
  .refs     = NULL,
  .upvals   = NULL
};

NSMap NameSpaces = {
  .type    = &NSMapType,
  .trace   = true,
  .free    = true,
  .sweep   = false,
  .gray    = true,
  
  .lf      = LF_625,
  .entries = NULL,
  .cnt     = 0,
  .maxc    = 0,
  .nts     = 0
};

SCache Strings = {
  .type    = &SCacheType,
  .trace   = false,
  .free    = true,
  .sweep   = false,
  .gray    = true,
  
  .lf      = LF_625,
  .entries = NULL,
  .cnt     = 0,
  .maxc    = 0,
  .nts     = 0
};
