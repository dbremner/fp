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
     * UNDEFINED generates the undefined object & returns it
     * NUMVAL generates a value for C of the correct type
     * CAR manipulates the object as a list & gives its first part
     * CDR is like CAR but gives all but the first
     * ISNUM provides a boolean saying if the named object is a number
     */
#define UNDEFINED return(obj_alloc(T_UNDEF));
#define NUMVAL(x) ( (x->o_type == T_INT) ? \
    ((x->o_val).o_int) : ((x->o_val).o_double) )
#define CAR(x) ( (x->o_val).o_list.car )
#define CDR(x) ( (x->o_val).o_list.cdr )
#define ISNUM(x) ( (x->o_type == T_INT) || (x->o_type == T_FLOAT) )

int numargs();

    /*
     * same()--looks at two objects and tells whether they are the same.
     *	We recurse if it is a list.
     */
static
same(struct object *o1, struct object *o2)
{
    if( o1 == o2 ) return( 1 );
    if( o1->o_type != o2->o_type ){
	if( o1->o_type == T_INT )
	    if( o2->o_type == T_FLOAT )
		return( o1->o_val.o_int == o2->o_val.o_double );
	if( o2->o_type == T_INT )
	    if( o1->o_type == T_FLOAT )
		return( o2->o_val.o_int == o1->o_val.o_double );
	return( 0 );
    }
    switch( o1->o_type ){
    case T_INT:
    case T_BOOL:
	return( o1->o_val.o_int == o2->o_val.o_int );
    case T_FLOAT:
	return( o1->o_val.o_double == o2->o_val.o_double );
    case T_LIST:
	return( same(CAR(o1),CAR(o2)) && same(CDR(o1),CDR(o2)) );
    default:
	fatal_err("Bad AST type in same()");
    exit(EXIT_FAILURE); //TODO
    }
    /*NOTREACHED*/
}

    /*
     * ispair()--tell if our argument object is a list of two elements
     */
static
ispair(struct object *obj)
{
    if( obj->o_type != T_LIST ) return( 0 );
    if( CAR(obj) == 0 ) return( 0 );
    if( CDR(obj) == 0 ) return( 0 );
    if( CDR(CDR(obj)) ) return( 0 );
    return( 1 );
}

    /*
     * eqobj()--tell if the two objects in the list are equal.
     *	undefined on ill-formed list, etc.
     */
struct object *
eqobj(struct object *obj)
{
    struct object *p;

    if( !ispair(obj) ){
	obj_unref(obj);
	UNDEFINED;
    }
    p = obj_alloc(T_BOOL);
    if( same(CAR(obj),CAR(CDR(obj))) )
	p->o_val.o_int = 1;
    else
	p->o_val.o_int = 0;
    obj_unref(obj);
    return(p);
}

    /*
     * noteqobj()--just like eqobj(), but not equal
     */
static struct object *
noteqobj(struct object *obj)
{
    struct object *p = eqobj(obj);

    if( p->o_type == T_BOOL )
	p->o_val.o_int = (p->o_val.o_int ? 0 : 1);
    return(p);
}

    /*
     * do_charfun()--execute the action of a binary function
     */
struct object *
do_charfun(struct ast *act, struct object *obj)
{
    struct object *p;
    double f;

    switch( (act->val).YYint ){

    case '=':
	return( eqobj(obj) );
    case NE:
	return( noteqobj(obj) );

    case '>':
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	case T_INT:
	    p = obj_alloc(T_BOOL);
	    (p->o_val).o_int = NUMVAL(CAR(obj)) > NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case GE:
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	case T_INT:
	    p = obj_alloc(T_BOOL);
	    (p->o_val).o_int = NUMVAL(CAR(obj)) >= NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case LE:
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	case T_INT:
	    p = obj_alloc(T_BOOL);
	    (p->o_val).o_int = NUMVAL(CAR(obj)) <= NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case '<':
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	case T_INT:
	    p = obj_alloc(T_BOOL);
	    (p->o_val).o_int = NUMVAL(CAR(obj)) < NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}

    case '+':
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	    p = obj_alloc(T_FLOAT);
	    (p->o_val).o_double = NUMVAL(CAR(obj))+NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	case T_INT:
	    p = obj_alloc(T_INT);
	    (p->o_val).o_int = NUMVAL(CAR(obj))+NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}
    case '-':
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	    p = obj_alloc(T_FLOAT);
	    (p->o_val).o_double = NUMVAL(CAR(obj))-NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	case T_INT:
	    p = obj_alloc(T_INT);
	    (p->o_val).o_int = NUMVAL(CAR(obj))-NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}
    case '*':
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	    p = obj_alloc(T_FLOAT);
	    (p->o_val).o_double = NUMVAL(CAR(obj))*NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	case T_INT:
	    p = obj_alloc(T_INT);
	    (p->o_val).o_int = NUMVAL(CAR(obj))*NUMVAL(CAR(CDR(obj)));
	    obj_unref(obj);
	    return(p);
	}
    case '/':
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED;
	case T_FLOAT:
	case T_INT:
	    f = NUMVAL(CAR(CDR(obj)));
	    if( f == 0.0 ){
		obj_unref(obj);
		UNDEFINED;
	    }
	    p = obj_alloc(T_FLOAT);
	    (p->o_val).o_double = NUMVAL(CAR(obj))/f;
	    obj_unref(obj);
	    return(p);
	}
    default:
	fatal_err("Undefined charop tag in execute()");
    exit(EXIT_FAILURE); //TODO
    }
    /*NOTREACHED*/
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
int
numargs(struct object *obj)
{
    struct object *p, *q;

	/*
	 * Don't have a well-formed list, so illegal
	 */
    if( !ispair(obj) ) return(T_UNDEF);

	/*
	 * So it's a list of two.  Verify type of both elements.
	 *	'p' gets the first object, 'q' gets second.
	 */
    p = CAR(obj);
    q = CAR(CDR(obj));
    if( !ISNUM(p) || !ISNUM(q) ) return(T_UNDEF);
    if( (p->o_type == T_FLOAT) || (q->o_type == T_FLOAT) )
	return(T_FLOAT);
    return(T_INT);
}
