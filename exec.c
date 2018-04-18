/*
 * Execution module for FP.  Runs along the AST and executes actions.
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"

    /*
     * This ugly set of macros makes access to objects easier.
     *
     * UNDEFINED generates the undefined object & returns it
     * NUMVAL generates a value for C of the correct type
     * CAR manipulates the object as a list & gives its first part
     * CDR is like CAR but gives all but the first
     * ISNUM provides a boolean saying if the named object is a number
     */
#define UNDEFINED return(obj_alloc(T_UNDEF));
#define NUMVAL(x) ( ((x)->o_type == T_INT) ? \
    (((x)->o_val).o_int) : (((x)->o_val).o_double) )
#define CAR(x) ( ((x)->o_val).o_list.car )
#define CDR(x) ( ((x)->o_val).o_list.cdr )
#define ISNUM(x) ( ((x)->o_type == T_INT) || (x->o_type == T_FLOAT) )

extern struct object *do_charfun(), *do_intrinsics();
static struct object *do_rinsert(), *do_binsert();

    /*
     * Given an AST for an action, and an object to do the action upon,
     *	execute the action and return the result.
     */
struct object *
execute( act, obj )
    register struct ast *act;
    register struct object *obj;
{
    register struct object *p, *q;
    int x;

	/*
	 * Broad categories of executable entities
	 */
    switch( act->tag ){

	/*
	 * Invoke a user-defined function
	 */
    case 'U':
	return( invoke( act->val.YYsym, obj) );

	/*
	 * Right-insert operator
	 */
    case '!':
	return( do_rinsert(act->left,obj) );

	/*
	 * Binary-insert operator
	 */
    case '|':
	return( do_binsert(act->left,obj) );

	/*
	 * Intrinsics
	 */
    case 'i':
	return( do_intrinsics(act->val.YYsym, obj) );

	/*
	 * Select one element from a list
	 */
    case 'S':
	if(
	    (obj->o_type != T_LIST) ||
	    !CAR(obj)
	){
	    obj_unref(obj);
	    UNDEFINED;
	}
	p = obj;
	if( (x = act->val.YYint) == 0 ){
	    obj_unref(obj);
	    UNDEFINED;
	}

	    /*
	     * Negative selectors count from end of list
	     */
	if( x < 0 ){
	    int tmp = listlen(p);

	    x += (tmp+1);
	    if( x < 0 ){
		obj_unref(obj);
		UNDEFINED;
	    }
	}
	while( --x ){		/* Scan down list X times */
	    if( !p ) break;
	    p = CDR(p);
	}
	if( !p ){		/* Fell off bottom of list */
	    obj_unref(obj);
	    UNDEFINED;
	}
	p = CAR(p);
	p->o_refs += 1;		/* Add reference to this elem */
	obj_unref(obj);		/* Unreference list as a whole */
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
	struct object *hd, **hdp = &hd;

	act = act->left;
	hd = (struct object *)0;
	while( act ){
	    obj->o_refs += 1;
	    if( (p = execute(act->left,obj))->o_type == T_UNDEF ){
		obj_unref(hd);
		obj_unref(obj);
		return(p);
	    }
	    *hdp = q = obj_alloc(T_LIST);
	    hdp = &(CDR(q));
	    CAR(q) = p;
	    act = act->right;
	}
	obj_unref(obj);
	return(hd);
    }

	/*
	 * These are the single-character operations (+, -, etc.)
	 */
    case 'c':
	return(do_charfun(act,obj));

	/*
	 * Conditional.  Evaluate & return one of the two paths
	 */
    case '>':
	obj->o_refs += 1;
	p = execute(act->left,obj);
	if( p->o_type == T_UNDEF ){
	    obj_unref(obj);
	    return(p);
	}
	if( p->o_type != T_BOOL ){
	    obj_unref(obj);
	    obj_unref(p);
	    UNDEFINED;
	}
	if( p->o_val.o_int ) q = execute(act->middle,obj);
	else q = execute(act->right,obj);
	obj_unref(p);
	return(q);

	/*
	 * Apply the action to each member of a list
	 */
    case '&': {
	struct object *hd, **hdp = &hd, *r;

	hd = 0;
	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED;
	}
	if( !CAR(obj) ) return(obj);
	for( p = obj; p; p = CDR(p) ){
	    (p->o_val.o_list.car)->o_refs += 1;
	    if( (q = execute(act->left,CAR(p)))->o_type == T_UNDEF ){
		obj_unref(hd); obj_unref(obj);
		return(q);
	    }
	    *hdp = r = obj_alloc(T_LIST);
	    CAR(r) = q;
	    hdp = &CDR(r);
	}
	obj_unref(obj);
	return(hd);
    }

	/*
	 * Introduce an object
	 */
    case '%':
	if( obj->o_type == T_UNDEF ) return(obj);
	obj_unref(obj);
	p = act->val.YYobj;
	p->o_refs += 1;
	return(p);
    
	/*
	 * Do a while loop
	 */
    case 'W':
	while( 1 ){
	    if( obj->o_type == T_UNDEF ){
		obj_unref(obj);
		UNDEFINED;
	    }
	    obj->o_refs += 1;
	    p = execute(act->left,obj);
	    if( p->o_type != T_BOOL ){
		obj_unref(obj);
		obj_unref(p);
		UNDEFINED;
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
    /*NOTREACHED*/
}

    /*
     * Local function to handle the tedious right-inserting
     */
static struct object *
do_rinsert(act,obj)
    struct ast *act;
    struct object *obj;
{
    register struct object *p, *q;

    if( obj->o_type != T_LIST ){
	obj_unref(obj);
	UNDEFINED;
    }

	/*
	 * If the list is empty, then we need to look at the applied
	 *	operator.  If it's one for which we have an identity,
	 *	return the identity.  Otherwise, undefined.  Bletch.
	 */
    if( !CAR(obj) ){
	obj_unref(obj);
	if( act->tag == 'c' ){
	    switch( act->val.YYint ){
	    case '+':
	    case '-':
		p = obj_alloc(T_INT);
		p->o_val.o_int = 0;
		break;
	    case '/':
	    case '*':
		p = obj_alloc(T_INT);
		p->o_val.o_int = 1;
		break;
	    default:
		UNDEFINED;
	    }
	} else if ( act->tag == 'i' ){
	    switch( (act->val.YYsym)->sym_val.YYint ){
	    case AND:
		p = obj_alloc(T_BOOL);
		p->o_val.o_int = 1;
		break;
	    case OR:
	    case XOR:
		p = obj_alloc(T_BOOL);
		p->o_val.o_int = 0;
		break;
	    default:
		UNDEFINED;
	    }
	} else UNDEFINED;
	return(p);
    }

	/*
	 * If the list has only one element, we return that element.
	 */
    if( !(p = CDR(obj)) ){
	p = CAR(obj);
	p->o_refs += 1;
	obj_unref(obj);
	return(p);
    }

	/*
	 * If the list has two elements, we apply our operator and reduce
	 */
    if( !CDR(p) ){
	return( execute(act,obj) );
    }

	/*
	 * Here's the nasty one.  We have three or more, so recurse on our-
	 *	selves to handle all but the first, then apply operation to
	 *	first linked onto the result.  Normal business over undefined
	 *	objects popping up.
	 */
    CDR(obj)->o_refs += 1;
    p = do_rinsert(act,CDR(obj));
    if( p->o_type == T_UNDEF ){
	obj_unref(obj);
	return(p);
    }
    q = obj_alloc(T_LIST);
    CAR(q) = CAR(obj);
    CAR(obj)->o_refs += 1;
    CAR(CDR(q) = obj_alloc(T_LIST)) = p;
    obj_unref(obj);
    return( execute(act,q) );
}

    /*
     * Local function to handle the tedious binary inserting
     */
static struct object *
do_binsert(act,obj)
    struct ast *act;
    struct object *obj;
{
    register struct object *p, *q;
    struct object *hd, **hdp, *r;
    int x;

    if( obj->o_type != T_LIST ){
	obj_unref(obj);
	UNDEFINED;
    }

	/*
	 * If the list is empty, then we need to look at the applied
	 *	operator.  If it's one for which we have an identity,
	 *	return the identity.  Otherwise, undefined.  Bletch.
	 */
    if( !CAR(obj) ){
	obj_unref(obj);
	if( act->tag == 'c' ){
	    switch( act->val.YYint ){
	    case '+':
	    case '-':
		p = obj_alloc(T_INT);
		p->o_val.o_int = 0;
		break;
	    case '/':
	    case '*':
		p = obj_alloc(T_INT);
		p->o_val.o_int = 1;
		break;
	    default:
		UNDEFINED;
	    }
	} else if ( act->tag == 'i' ){
	    switch( (act->val.YYsym)->sym_val.YYint ){
	    case AND:
		p = obj_alloc(T_BOOL);
		p->o_val.o_int = 1;
		break;
	    case OR:
	    case XOR:
		p = obj_alloc(T_BOOL);
		p->o_val.o_int = 0;
		break;
	    default:
		UNDEFINED;
	    }
	} else UNDEFINED;
	return(p);
    }

	/*
	 * If the list has only one element, we return that element.
	 */
    if( !(p = CDR(obj)) ){
	p = CAR(obj);
	p->o_refs += 1;
	obj_unref(obj);
	return(p);
    }

	/*
	 * If the list has two elements, we apply our operator and reduce
	 */
    if( !CDR(p) ){
	return( execute(act,obj) );
    }

	/*
	 * For three or more elements, we must set up to split the list
	 *	into halves.  For every two steps which 'p' makes forward,
	 *	'q' advances one.  When 'p' hits the end, 'q' names the 2nd
	 *	half, and 'hd' names a copy of the first.
	 */
    x = 0;
    hd = 0;
    hdp = &hd;
    for( q = obj; p; p = CDR(p) ){
	if( x ){
	    *hdp = r = obj_alloc(T_LIST);
	    hdp = &CDR(r);
	    CAR(r) = CAR(q);
	    CAR(q)->o_refs += 1;
	    q = CDR(q);
	    x = 0;
	} else
	    x = 1;
    }
    *hdp = p = obj_alloc(T_LIST);
    CAR(p) = CAR(q);
    CAR(q)->o_refs += 1;

	/*
	 * 'q' names the second half, but we must add a reference, otherwise
	 *	our use of it via execute() will consume it (and obj still
	 *	references it...).
	 */
    q = CDR(q);
    q->o_refs += 1;

	/*
	 * Almost there... "hd" is the first, "q" is the second, we encase
	 *	them in an outer list, and call execute on them.
	 */
    p = obj_alloc(T_LIST);
    CAR(p) = do_binsert(act,hd);
    CAR(CDR(p) = obj_alloc(T_LIST)) = do_binsert(act,q);
    obj_unref(obj);
    return(execute(act,p));
}
