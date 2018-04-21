/*
 * defun.c--define a user function
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "symtab.h"
#include <stdio.h>

/// Define a function
void
defun(sym_ptr name, ast_ptr def)
{
	// Check what we're defining, handle redefining
    switch( name->sym_type ){
    case symtype::SYM_DEF:
	printf("%s: redefined.\n",name->sym_pname.c_str());
	ast_freetree(name->sym_val.YYast);
	break;
    case symtype::SYM_NEW:
	printf("{%s}\n",name->sym_pname.c_str());
	break;
    default:
	fatal_err("Bad symbol stat in defun()");
    }

	/*
	 * Mark symbol as a user-defined function, attach its
	 *	definition.
	 */
    name->sym_val.YYast = def;
    name->sym_type = symtype::SYM_DEF;
}

/// Call a previously-defined user function, or error
obj_ptr
invoke(sym_ptr def, obj_ptr obj)
{
	// Must be a defined function
    if( def->sym_type != symtype::SYM_DEF ){
	printf("%s: undefined\n",def->sym_pname.c_str());
	obj_unref(obj);
	return( obj_alloc(obj_type::T_UNDEF) );
    }

	// Call it with the object
    return( execute( def->sym_val.YYast, obj ) );
}
