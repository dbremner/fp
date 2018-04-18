/*
 * defun.c--define a user function
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "symtab.h"

    /*
     * Define a function
     */
void
defun(name,def)
    struct symtab *name;
    struct ast *def;
{
	/*
	 * Check what we're defining, handle redefining
	 */
    switch( name->sym_type ){
    case SYM_DEF:
	printf("%s: redefined.\n",name->sym_pname);
	ast_freetree(name->sym_val.YYast);
	break;
    case SYM_NEW:
	printf("{%s}\n",name->sym_pname);
	break;
    default:
	fatal_err("Bad symbol stat in defun()");
    }

	/*
	 * Mark symbol as a user-defined function, attach its
	 *	definition.
	 */
    name->sym_val.YYast = def;
    name->sym_type = SYM_DEF;
}

    /*
     * Call a previously-defined user function, or error
     */
struct object *
invoke( def, obj )
    struct symtab *def;
    struct object *obj;
{
	/*
	 * Must be a defined function
	 */
    if( def->sym_type != SYM_DEF ){
	printf("%s: undefined\n",def->sym_pname);
	obj_unref(obj);
	return( obj_alloc(T_UNDEF) );
    }

	/*
	 * Call it with the object
	 */
    return( execute( def->sym_val.YYast, obj ) );
}
