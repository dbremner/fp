/*
 * defun.c--define a user function
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <stdio.h>
#include "fpcommon.h"
#include "ast.h"
#include "defun.h"
#include "misc.h"
#include "yystype.h"
#include "symtab_entry.hpp"

/// Define a function
void
defun(sym_ptr name, live_ast_ptr def)
{
    assert(name);
    assert(def);
	// Check what we're defining, handle redefining
    switch( name->sym_type ){
        case symtype::SYM_DEF:
            printf("%s: redefined.\n",name->sym_pname.c_str());
            ast_freetree(name->sym_val.YYast);
            break;
        case symtype::SYM_NEW:
            printf("{%s}\n",name->sym_pname.c_str());
            break;
        case symtype::SYM_BUILTIN:
            fatal_err("Bad symbol stat in defun()");
    }

	/*
	 * Mark symbol as a user-defined function, attach its
	 *	definition.
	 */
    name->sym_val.YYast = def;
    name->sym_type = symtype::SYM_DEF;
}
