#include <stdio.h>
#include "fpcommon.h"
#include "ast.h"
#include "defun.h"
#include "misc.h"
#include "yystype.h"
#include "symtab_entry.hpp"

/// Define a function
void symtab_entry::define(live_ast_ptr def)
{
    assert(def);
    // Check what we're defining, handle redefining
    switch( sym_type ){
        case symtype::SYM_DEF:
            printf("%s: redefined.\n", sym_pname.c_str());
            ast_freetree(sym_val.YYast);
            break;
        case symtype::SYM_NEW:
            printf("{%s}\n", sym_pname.c_str());
            break;
        case symtype::SYM_BUILTIN:
            fatal_err("Bad symbol stat in defun()");
    }
    
    /*
     * Mark symbol as a user-defined function, attach its
     *    definition.
     */
    sym_val.YYast = def;
    sym_type = symtype::SYM_DEF;
}
