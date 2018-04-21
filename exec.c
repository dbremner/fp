/*
 * Execution module for FP.  Runs along the AST and executes actions.
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "object_inlines.hpp"
#include "y.tab.h"

    /*
     * This ugly set of macros makes access to objects easier.
     *
     * CAR manipulates the object as a list & gives its first part
     * CDR is like CAR but gives all but the first
     */
#define CAR(x) ( ((x)->o_val).o_list.car )
#define CDR(x) ( ((x)->o_val).o_list.cdr )

static obj_ptr invoke(sym_ptr def, obj_ptr obj);
static obj_ptr do_rinsert(ast_ptr act, obj_ptr obj);
static obj_ptr do_binsert(ast_ptr act, obj_ptr obj);

    /*
     * Given an AST for an action, and an object to do the action upon,
     *	execute the action and return the result.
     */
obj_ptr
execute(ast_ptr act, obj_ptr obj )
{
    obj_ptr p;
    obj_ptr q;
    int x;

	// Broad categories of executable entities
    switch( act->tag ){

	// Invoke a user-defined function
    case 'U':
	return( invoke( act->val.YYsym, obj) );

	// Right-insert operator
    case '!':
	return( do_rinsert(act->left,obj) );

	// Binary-insert operator
    case '|':
	return( do_binsert(act->left,obj) );

	// Intrinsics
    case 'i':
	return( do_intrinsics(act->val.YYsym, obj) );

	// Select one element from a list
    case 'S':
	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    !CAR(obj)
	){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj;
	if( (x = act->val.YYint) == 0 ){
	    obj_unref(obj);
	    return undefined();
	}

	    // Negative selectors count from end of list
	if( x < 0 ){
	    int tmp = listlen(p);

	    x += (tmp+1);
	    if( x < 0 ){
		obj_unref(obj);
		return undefined();
	    }
	}
	while( --x ){		// Scan down list X times
	    if( !p ) break;
	    p = p->cdr();
	}
	if( !p ){		// Fell off bottom of list
	    obj_unref(obj);
	    return undefined();
	}
	p = p->car();
	p->inc_ref();		// Add reference to this elem
	obj_unref(obj);		// Unreference list as a whole
	return(p);

	/*
	 * Apply the action on the left to the result of executing
	 *	the action on the right against the object.
	 */
    case '@':
	p = execute( act->right, obj );
	return( execute( act->left, p ) );

	/*
	 * Build a new list by applying the listed actions to the object
	 *	All is complicated by the fact that we must be clean in
	 *	the presence of T_UNDEF popping up along the way.
	 */
    case '[':{
    obj_ptr hd;
    obj_ptr *hdp = &hd;

	act = act->left;
	hd = nullptr;
	while( act ){
	    obj->inc_ref();
	    if( (p = execute(act->left,obj))->o_type == obj_type::T_UNDEF ){
		obj_unref(hd);
		obj_unref(obj);
		return(p);
	    }
	    *hdp = q = obj_alloc(obj_type::T_LIST);
	    hdp = &(CDR(q));
	    CAR(q) = p;
	    act = act->right;
	}
	obj_unref(obj);
	return(hd);
    }

	// These are the single-character operations (+, -, etc.)
    case 'c':
	return(do_charfun(act,obj));

	// Conditional.  Evaluate & return one of the two paths
    case '>':
	obj->inc_ref();
	p = execute(act->left,obj);
	if( p->o_type == obj_type::T_UNDEF ){
	    obj_unref(obj);
	    return(p);
	}
	if( p->o_type != obj_type::T_BOOL ){
	    obj_unref(obj);
	    obj_unref(p);
	    return undefined();
	}
	if( p->o_val.o_int ) q = execute(act->middle,obj);
	else q = execute(act->right,obj);
	obj_unref(p);
	return(q);

	// Apply the action to each member of a list
    case '&': {
    obj_ptr hd;
    obj_ptr *hdp = &hd;
    obj_ptr r;

	hd = nullptr;
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !obj->car() ) return(obj);
	for( p = obj; p; p = p->cdr() ){
	    (p->o_val.o_list.car)->inc_ref();
	    if( (q = execute(act->left,car_(p)))->o_type == obj_type::T_UNDEF ){
		obj_unref(hd); obj_unref(obj);
		return(q);
	    }
	    *hdp = r = obj_alloc(obj_type::T_LIST);
	    CAR(r) = q;
	    hdp = &CDR(r);
	}
	obj_unref(obj);
	return(hd);
    }

	// Introduce an object
    case '%':
	if( obj->o_type == obj_type::T_UNDEF ) return(obj);
	obj_unref(obj);
	p = act->val.YYobj;
	p->inc_ref();
	return(p);
    
	// Do a while loop
    case 'W':
	while( 1 ){
	    if( obj->o_type == obj_type::T_UNDEF ){
		obj_unref(obj);
		return undefined();
	    }
	    obj->inc_ref();
	    p = execute(act->left,obj);
	    if( p->o_type != obj_type::T_BOOL ){
		obj_unref(obj);
		obj_unref(p);
		return undefined();
	    }
	    if( p->o_val.o_int ){
		obj_unref(p);
		obj = execute(act->right,obj);
	    } else {
		obj_unref(p);
		return(obj);
	    }
	}

    default:
	fatal_err("Undefined AST tag in execute()");
    }
}

/// Call a previously-defined user function, or error
static obj_ptr
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

/// Local function to handle the tedious right-inserting
static obj_ptr
do_rinsert(ast_ptr act, obj_ptr obj)
{
    obj_ptr p;
    obj_ptr q;

    if( obj->o_type != obj_type::T_LIST ){
	obj_unref(obj);
	return undefined();
    }

	/*
	 * If the list is empty, then we need to look at the applied
	 *	operator.  If it's one for which we have an identity,
	 *	return the identity.  Otherwise, undefined.  Bletch.
	 */
    if( !obj->car() ){
	obj_unref(obj);
	if( act->tag == 'c' ){
	    switch( act->val.YYint ){
	    case '+':
	    case '-':
		p = obj_alloc(obj_type::T_INT);
		p->o_val.o_int = 0;
		break;
	    case '/':
	    case '*':
		p = obj_alloc(obj_type::T_INT);
		p->o_val.o_int = 1;
		break;
	    default:
		return undefined();
	    }
	} else if ( act->tag == 'i' ){
	    switch( (act->val.YYsym)->sym_val.YYint ){
	    case AND:
		p = obj_alloc(obj_type::T_BOOL);
		p->o_val.o_int = 1;
		break;
	    case OR:
	    case XOR:
		p = obj_alloc(obj_type::T_BOOL);
		p->o_val.o_int = 0;
		break;
	    default:
		return undefined();
	    }
	} else return undefined();
	return(p);
    }

	// If the list has only one element, we return that element.
    if( !(p = obj->cdr()) ){
	p = obj->car();
	p->inc_ref();
	obj_unref(obj);
	return(p);
    }

	// If the list has two elements, we apply our operator and reduce
    if( !p->cdr() ){
	return( execute(act,obj) );
    }

	/*
	 * Here's the nasty one.  We have three or more, so recurse on our-
	 *	selves to handle all but the first, then apply operation to
	 *	first linked onto the result.  Normal business over undefined
	 *	objects popping up.
	 */
    obj->cdr()->inc_ref();
    p = do_rinsert(act,obj->cdr());
    if( p->o_type == obj_type::T_UNDEF ){
	obj_unref(obj);
	return(p);
    }
    q = obj_alloc(obj_type::T_LIST);
    CAR(q) = car_(obj);
    obj->car()->inc_ref();
    CAR(CDR(q) = obj_alloc(obj_type::T_LIST)) = p;
    obj_unref(obj);
    return( execute(act,q) );
}

/// Local function to handle the tedious binary inserting
static obj_ptr
do_binsert(ast_ptr act, obj_ptr obj)
{
    obj_ptr p;
    obj_ptr q;
    obj_ptr hd;
    obj_ptr *hdp;
    obj_ptr r;
    int x;

    if( obj->o_type != obj_type::T_LIST ){
	obj_unref(obj);
	return undefined();
    }

	/*
	 * If the list is empty, then we need to look at the applied
	 *	operator.  If it's one for which we have an identity,
	 *	return the identity.  Otherwise, undefined.  Bletch.
	 */
    if( !obj->car() ){
	obj_unref(obj);
	if( act->tag == 'c' ){
	    switch( act->val.YYint ){
	    case '+':
	    case '-':
		p = obj_alloc(obj_type::T_INT);
		p->o_val.o_int = 0;
		break;
	    case '/':
	    case '*':
		p = obj_alloc(obj_type::T_INT);
		p->o_val.o_int = 1;
		break;
	    default:
		return undefined();
	    }
	} else if ( act->tag == 'i' ){
	    switch( (act->val.YYsym)->sym_val.YYint ){
	    case AND:
		p = obj_alloc(obj_type::T_BOOL);
		p->o_val.o_int = 1;
		break;
	    case OR:
	    case XOR:
		p = obj_alloc(obj_type::T_BOOL);
		p->o_val.o_int = 0;
		break;
	    default:
		return undefined();
	    }
	} else return undefined();
	return(p);
    }

	// If the list has only one element, we return that element.
    if( !(p = obj->cdr()) ){
	p = obj->car();
	p->inc_ref();
	obj_unref(obj);
	return(p);
    }

	// If the list has two elements, we apply our operator and reduce
    if( !p->cdr() ){
	return( execute(act,obj) );
    }

	/*
	 * For three or more elements, we must set up to split the list
	 *	into halves.  For every two steps which 'p' makes forward,
	 *	'q' advances one.  When 'p' hits the end, 'q' names the 2nd
	 *	half, and 'hd' names a copy of the first.
	 */
    x = 0;
    hd = nullptr;
    hdp = &hd;
    for( q = obj; p; p = p->cdr() ){
	if( x ){
	    *hdp = r = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(r);
	    CAR(r) = q->car();
	    q->car()->inc_ref();
	    q = q->cdr();
	    x = 0;
	} else
	    x = 1;
    }
    *hdp = p = obj_alloc(obj_type::T_LIST);
    CAR(p) = q->car();
    q->car()->inc_ref();

	/*
	 * 'q' names the second half, but we must add a reference, otherwise
	 *	our use of it via execute() will consume it (and obj still
	 *	references it...).
	 */
    q = q->cdr();
    q->inc_ref();

	/*
	 * Almost there... "hd" is the first, "q" is the second, we encase
	 *	them in an outer list, and call execute on them.
	 */
    p = obj_alloc(obj_type::T_LIST);
    CAR(p) = do_binsert(act,hd);
    CAR(CDR(p) = obj_alloc(obj_type::T_LIST)) = do_binsert(act,q);
    obj_unref(obj);
    return(execute(act,p));
}
