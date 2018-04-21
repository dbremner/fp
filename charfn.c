/*
 * charfn.c--functions to do the "character" functions, like +, -, ...
 *
 * 	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"

    /*
     * This ugly set of macros makes access to objects easier.
     *
     * NUMVAL generates a value for C of the correct type
     * CAR manipulates the object as a list & gives its first part
     */

#define NUMVAL(x) ( (x->o_type == obj_type::T_INT) ? \
    ((x->o_val).o_int) : ((x->o_val).o_double) )

    /*
     * same()--looks at two objects and tells whether they are the same.
     *	We recurse if it is a list.
     */
static bool
same(obj_ptr o1, obj_ptr o2)
{
    if( o1 == o2 ) return(true);
    if( o1->o_type != o2->o_type ){
	if( o1->o_type == obj_type::T_INT )
	    if( o2->o_type == obj_type::T_FLOAT )
		return( o1->o_val.o_int == o2->o_val.o_double );
	if( o2->o_type == obj_type::T_INT )
	    if( o1->o_type == obj_type::T_FLOAT )
		return( o2->o_val.o_int == o1->o_val.o_double );
	return(false);
    }
    switch( o1->o_type ){
    case obj_type::T_INT:
    case obj_type::T_BOOL:
	return( o1->o_val.o_int == o2->o_val.o_int );
    case obj_type::T_FLOAT:
	return( o1->o_val.o_double == o2->o_val.o_double );
    case obj_type::T_LIST:
	return( same(car_(o1),car_(o2)) && same(cdr_(o1),cdr_(o2)) );
    default:
	fatal_err("Bad AST type in same()");
    }
}

/// ispair()--tell if our argument object is a list of two elements
static bool
ispair(obj_ptr obj)
{
    if( obj->o_type != obj_type::T_LIST ) return(false);
    if( car_(obj) == nullptr ) return(false);
    if( cdr_(obj) == nullptr ) return(false);
    if( cdr_(cdr_(obj)) ) return(false);
    return(true);
}

    /*
     * eqobj()--tell if the two objects in the list are equal.
     *	undefined on ill-formed list, etc.
     */
obj_ptr
eqobj(obj_ptr obj)
{
    obj_ptr p;

    if( !ispair(obj) ){
	obj_unref(obj);
	return undefined();
    }
    p = obj_alloc(obj_type::T_BOOL);
    if( same(car_(obj),car_(cdr_(obj))) )
	p->o_val.o_int = 1;
    else
	p->o_val.o_int = 0;
    obj_unref(obj);
    return(p);
}

/// noteqobj()--just like eqobj(), but not equal
static obj_ptr
noteqobj(obj_ptr obj)
{
    obj_ptr p = eqobj(obj);

    if( p->o_type == obj_type::T_BOOL )
	p->o_val.o_int = (p->o_val.o_int ? 0 : 1);
    return(p);
}

/// do_charfun()--execute the action of a binary function
obj_ptr
do_charfun(ast_ptr act, obj_ptr obj)
{
    obj_ptr p;
    double f;

    switch( (act->val).YYint ){

    case '=':
	return( eqobj(obj) );
    case NE:
	return( noteqobj(obj) );

    case '>':
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_BOOL);
	    (p->o_val).o_int = NUMVAL(car_(obj)) > NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case GE:
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_BOOL);
	    (p->o_val).o_int = NUMVAL(car_(obj)) >= NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case LE:
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_BOOL);
	    (p->o_val).o_int = NUMVAL(car_(obj)) <= NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case '<':
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_BOOL);
	    (p->o_val).o_int = NUMVAL(car_(obj)) < NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case '+':
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	    p = obj_alloc(obj_type::T_FLOAT);
	    (p->o_val).o_double = NUMVAL(car_(obj))+NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = NUMVAL(car_(obj))+NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}
    case '-':
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	    p = obj_alloc(obj_type::T_FLOAT);
	    (p->o_val).o_double = NUMVAL(car_(obj))-NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = NUMVAL(car_(obj))-NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}
    case '*':
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	    p = obj_alloc(obj_type::T_FLOAT);
	    (p->o_val).o_double = NUMVAL(car_(obj))*NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	case obj_type::T_INT:
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = NUMVAL(car_(obj))*NUMVAL(car_(cdr_(obj)));
	    obj_unref(obj);
	    return(p);
	}
    case '/':
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:
	    f = NUMVAL(car_(cdr_(obj)));
	    if( f == 0.0 ){
		obj_unref(obj);
		return undefined();
	    }
	    p = obj_alloc(obj_type::T_FLOAT);
	    (p->o_val).o_double = NUMVAL(car_(obj))/f;
	    obj_unref(obj);
	    return(p);
	}
    default:
	fatal_err("Undefined charop tag in execute()");
    }
}

    /*
     * numargs()--process a list which is to be used as a pair of numeric
     *	arguments to a function.
     *
     *	+, -, /, etc.  all need two functions:  first, they need to know
     *	if their arguments are OK.  Is it a list, are there two
     *	numbers in it?, etc.  We make C normalize the two numbers, but
     *	we tell our caller if the result will be double or int, so that he
     *	can allocate the right type of object.
     */
obj_type
numargs(obj_ptr obj)
{
    obj_ptr p;
    obj_ptr q;

	// Don't have a well-formed list, so illegal
    if( !ispair(obj) ) return(obj_type::T_UNDEF);

	/*
	 * So it's a list of two.  Verify type of both elements.
	 *	'p' gets the first object, 'q' gets second.
	 */
    p = car_(obj);
    q = car_(cdr_(obj));
    if( !p->is_num() || !q->is_num() ) return(obj_type::T_UNDEF);
    if( (p->o_type == obj_type::T_FLOAT) || (q->o_type == obj_type::T_FLOAT) )
	return(obj_type::T_FLOAT);
    return(obj_type::T_INT);
}
