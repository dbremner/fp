/*
 * Yet another symbol tabler
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"

#define SYMTABSIZE 101

    /*
     * sym_type values
     */

enum symtype {
    SYM_BUILTIN = 1, /* A built-in */
    SYM_DEF = 2, /* User-defined */
    SYM_NEW = 3 /* Never seen before! */
};

