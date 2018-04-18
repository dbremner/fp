/*
 * intrin.c--intrinsic functions for FP.  These are the ones which
 *	parse as an identifier, and are symbol-tabled.
 *
 * 	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"
#include "math.h"

    /*
     * This ugly set of macros makes access to objects easier.
     *
     * UNDEFINED() generates the undefined object & returns it
     * NUMVAL generates a value for C of the correct type
     * CAR manipulates the object as a list & gives its first part
     * CDR is like CAR but gives all but the first
     * ISNUM provides a boolean saying if the named object is a number
     */
#define UNDEFINED() return(obj_alloc(T_UNDEF));
#define NUMVAL(x) ( (x->o_type == T_INT) ? \
    ((x->o_val).o_int) : ((x->o_val).o_double) )
#define CAR(x) ( ((x)->o_val).o_list.car )
#define CDR(x) ( ((x)->o_val).o_list.cdr )
#define ISNUM(x) ( (x->o_type == T_INT) || (x->o_type == T_FLOAT) )

static struct object *
do_dist(struct object *elem, struct object *lst, struct object *obj, int side);

static struct object *do_trans(struct object *obj);
static struct object *do_bool(struct object *obj, int op);

    /*
     * Main intrinsic processing routine
     */
struct object *
do_intrinsics(struct symtab *act, struct object *obj)
{
    struct object *p, *q;
    double f;

	/*
	 * Switch off the tokenal value assigned by YACC.  Depending on the
	 *	sophistication of your C compiler, this can generate some
	 *	truly horrendous code.  Be prepared!  Perhaps it would be
	 *	better to store a pointer to a function in with the symbol
	 *	table...
	 */
    switch( act->sym_val.YYint ){

    case LENGTH:{	/* Length of a list */
	int l;

	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	for( p = obj, l = 0; p && CAR(p); p = CDR(p) ) l++;
	obj_unref(obj);
	p = obj_alloc(T_INT);
	p->o_val.o_int = l;
	return(p);
    }

    case ID:		/* Identity */
	return(obj);
    case OUT:		/* Identity, but print debug line too */
	printf("out: ");
	obj_prtree(obj);
	putchar('\n');
	return(obj);
    
    case FIRST:
    case HD:		/* First elem of a list */
	if( obj->o_type != T_LIST ){
	    obj_unref(obj); UNDEFINED();
	}
	if( !(p = CAR(obj)) ) return(obj);
	p->o_refs += 1;
	obj_unref(obj);
	return(p);

    case TL:		/* Remainder of list */
	if( (obj->o_type != T_LIST) || !CAR(obj) ){
	    obj_unref(obj); UNDEFINED();
	}
	if( !(p = CDR(obj)) ){
	    p = obj_alloc(T_LIST);
	} else {
	    p->o_refs += 1;
	}
	obj_unref(obj);
	return(p);

    case IOTA:{		/* Given arg N, generate <1..N> */
	int x, l;
	struct object *hd, **hdp = &hd;

	if( (obj->o_type != T_INT) && (obj->o_type != T_FLOAT) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	l = (obj->o_type == T_INT) ? obj->o_val.o_int : obj->o_val.o_double;
	obj_unref(obj);
	if( l < 0 ) UNDEFINED();
	if( l == 0 ) return( obj_alloc(T_LIST) );
	for( x = 1; x <= l; x++ ){
	    *hdp = p = obj_alloc(T_LIST);
	    q = obj_alloc(T_INT);
	    q->o_val.o_int = x;
	    CAR(p) = q;
	    hdp = &CDR(p);
	}
	return(hd);
    } /* Local block for IOTA */

    case PICK:{		/* Parameterized selection */
	int x;

	    /*
	     * Verify all elements which we will use
	     */
	if(
	    (obj->o_type != T_LIST) ||
	    ( (p = CAR(obj))->o_type != T_INT ) ||
	    !(q = CDR(obj)) ||
	    ( (q = CAR(q))->o_type != T_LIST) ||
	    ( (x = p->o_val.o_int) == 0 )
	){
	    obj_unref(obj);
	    UNDEFINED();
	}

	    /*
	     * If x is negative, we are counting from the end
	     */
	if( x < 0 ){
	    int tmp = listlen(q);

	    x += (tmp + 1);
	    if( x < 1 ){
		obj_unref(obj);
		UNDEFINED();
	    }
	}

	    /*
	     * Loop along the list until our count is expired
	     */
	for( ; x > 1; --x ){
	    if( !q ) break;
	    q = CDR(q);
	}

	    /*
	     * If fell off the list, error
	     */
	if( !q || !(q = CAR(q)) ){
	    obj_unref(obj);
	    UNDEFINED();
	}

	    /*
	     * Add a reference to the named object, release the old object
	     */
	q->o_refs += 1;
	obj_unref(obj);
	return(q);
    }

    case LAST:		/* Return last element of list */
	if( (q = obj)->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	if( !CAR(obj) ) return(obj);
    while( (p = CDR(q)) ) q = p;
	q = CAR(q);
	q->o_refs += 1;
	obj_unref(obj);
	return(q);
    
    case FRONT:
    case TLR:{		/* Return a list of all but list */
	struct object *hd = 0, **hdp = &hd;

	if(
	    ((q = obj)->o_type != T_LIST) ||
	    !CAR(obj)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	while( CDR(q) ){
	    *hdp = p = obj_alloc(T_LIST);
        if( (CAR(p) = CAR(q)) ){
		CAR(p)->o_refs += 1;
	    }
	    hdp = &CDR(p);
	    q = CDR(q);
	}
	obj_unref(obj);
	if( !hd ) return( obj_alloc(T_LIST) );
	else return(hd);
    }

    case DISTL:		/* Distribute from left-most element */
	if(
	    (obj->o_type != T_LIST) ||
	    ( !(q = CAR(obj)) ) ||
	    (!CDR(obj)) ||
	    (!(p = CAR(CDR(obj))) ) ||
	    (p->o_type != T_LIST)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	return( do_dist(q,p,obj,0) );

    case DISTR:		/* Distribute from left-most element */
	if(
	    (obj->o_type != T_LIST) ||
	    ( !(q = CAR(obj)) ) ||
	    (!CDR(obj)) ||
	    (!(p = CAR(CDR(obj))) ) ||
	    (q->o_type != T_LIST)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	return( do_dist(p,q,obj,1) );
    
    case APNDL:{	/* Append element from left */
	struct object *r;

	if(
	    (obj->o_type != T_LIST) ||
	    ( !(q = CAR(obj)) ) ||
	    (!CDR(obj)) ||
	    (!(p = CAR(CDR(obj))) ) ||
	    (p->o_type != T_LIST)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	q->o_refs += 1;
	if( !CAR(p) ){		/* Null list? */
	    obj_unref(obj);
	    p = obj_alloc(T_LIST);
	    CAR(p) = q;
	    return(p);		/* Just return element */
	}
	p->o_refs += 1;
	r = obj_alloc(T_LIST);
	CDR(r) = p;
	CAR(r) = q;
	obj_unref(obj);
	return(r);
    }

    case APNDR:{	/* Append element from right */
	struct object *hd = 0, **hdp = &hd, *r;

	if(
	    (obj->o_type != T_LIST) ||
	    ( !(q = CAR(obj)) ) ||
	    (!CDR(obj)) ||
	    (!(r = CAR(CDR(obj))) ) ||
	    (q->o_type != T_LIST)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	r->o_refs += 1;
	if( !CAR(q) ){		/* Empty list */
	    obj_unref(obj);
	    p = obj_alloc(T_LIST);
	    CAR(p) = r;
	    return(p);		/* Just return elem */
	}

	    /*
	     * Loop through list, building a new one.  We can't just reuse
	     *	the old one because we're modifying its end.
	     */
	while( q ){
	    *hdp = p = obj_alloc(T_LIST);
	    CAR(q)->o_refs += 1;
	    CAR(p) = CAR(q);
	    hdp = &CDR(p);
	    q = CDR(q);
	}

	    /*
	     * Tack the element onto the end of the built list
	     */
	*hdp = p = obj_alloc(T_LIST);
	CAR(p) = r;
	obj_unref(obj);
	return(hd);
    }

    case TRANS:		/* Transposition */
	return( do_trans(obj) );
    
    case REVERSE:{	/* Reverse all elements of a list */
	struct object *r;

	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	if( !CAR(obj) ) return(obj);
	for( p = 0, q = obj; q; q = CDR(q) ){
	    r = obj_alloc(T_LIST);
	    CDR(r) = p;
	    p = r;
	    CAR(p) = CAR(q);
	    CAR(q)->o_refs += 1;
	}
	obj_unref(obj);
	return(p);
    }

    case ROTL:{		/* Rotate left */
	struct object *hd = 0, **hdp = &hd;

	    /*
	     * Wanna list
	     */
	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}

	    /*
	     * Need two elems, otherwise be ID function
	     */
	if(
	    !(CAR(obj)) ||
	    !(q = CDR(obj)) ||
	    !(CAR(q))
	){
	    return(obj);
	}

	    /*
	     * Loop, starting from second.  Build parallel list.
	     */
	for( /* q has CDR(obj) */ ; q; q = CDR(q) ){
	    *hdp = p = obj_alloc(T_LIST);
	    hdp = &CDR(p);
	    CAR(p) = CAR(q);
	    CAR(q)->o_refs += 1;
	}
	*hdp = p = obj_alloc(T_LIST);
	CAR(p) = CAR(obj);
	CAR(obj)->o_refs += 1;
	obj_unref(obj);
	return(hd);
    }

    case ROTR:{		/* Rotate right */
	struct object *hd = 0, **hdp = &hd;

	    /*
	     * Wanna list
	     */
	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}

	    /*
	     * Need two elems, otherwise be ID function
	     */
	if(
	    !(CAR(obj)) ||
	    !(q = CDR(obj)) ||
	    !(CAR(q))
	){
	    return(obj);
	}

	    /*
	     * Loop over list.  Stop one short of end.
	     */
	for( q = obj; CDR(q); q = CDR(q) ){
	    *hdp = p = obj_alloc(T_LIST);
	    hdp = &CDR(p);
	    CAR(p) = CAR(q);
	    CAR(q)->o_refs += 1;
	}
	p = obj_alloc(T_LIST);
	CAR(p) = CAR(q);
	CAR(q)->o_refs += 1;
	CDR(p) = hd;
	obj_unref(obj);
	return(p);
    }

    case CONCAT:{		/* Concatenate several lists */
	struct object *hd = 0, **hdp = &hd, *r;

	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	if( !CAR(obj) ) return(obj);
	for( p = obj; p; p = CDR(p) ){
	    q = CAR(p);
	    if( q->o_type != T_LIST ){
		obj_unref(obj);
		obj_unref(hd);
		UNDEFINED();
	    }
	    if( !CAR(q) ) continue;
	    for( ; q; q = CDR(q) ){
		*hdp = r = obj_alloc(T_LIST);
		hdp = &CDR(r);
		CAR(r) = CAR(q);
		CAR(q)->o_refs += 1;
	    }
	}
	obj_unref(obj);
	if( !hd )
	    return(obj_alloc(T_LIST));
	return(hd);
    }

    case SIN:		/* sin() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = sin(f);
	obj_unref(obj);
	return(p);

    case COS:		/* cos() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = cos(f);
	obj_unref(obj);
	return(p);

    case TAN:		/* tan() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = tan(f);
	obj_unref(obj);
	return(p);

    case ASIN:		/* asin() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = asin(f);
	obj_unref(obj);
	return(p);

    case ACOS:		/* acos() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = acos(f);
	obj_unref(obj);
	return(p);

    case ATAN:		/* atan() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = atan(f);
	obj_unref(obj);
	return(p);
    
    case EXP:		/* exp() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = exp(f);
	obj_unref(obj);
	return(p);
    
    case LOG:		/* log() function */
	if( !ISNUM(obj) ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = log(f);
	obj_unref(obj);
	return(p);
    
    case MOD:		/* Modulo */
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED();
	case T_FLOAT:
	case T_INT:{
	    int x1, x2;

	    x1 = NUMVAL(CAR(obj));
	    if( (x2 = NUMVAL(CAR(CDR(obj)))) == 0 ){
		obj_unref(obj);
		UNDEFINED();
	    }
	    p = obj_alloc(T_INT);
	    (p->o_val).o_int = x1 % x2;
	    obj_unref(obj);
	    return(p);
	}
	}
    
    case PAIR:{		/* Pair up successive elements of a list */
	struct object *hd = 0, **hdp = &hd, *r;
	int x;

	if(
	    (obj->o_type != T_LIST) ||
	    !CAR(obj)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	for( p = obj, x = 0; p; p = CDR(p) ){
	    if( x == 0 ){
		*hdp = q = obj_alloc(T_LIST);
		hdp = &CDR(q);
		CAR(q) = r = obj_alloc(T_LIST);
		CAR(r) = CAR(p);
		CAR(p)->o_refs += 1;
		x++;
	    } else {
		CDR(r) = q = obj_alloc(T_LIST);
		CAR(q) = CAR(p);
		CAR(p)->o_refs += 1;
		x = 0;
	    }
	}
	obj_unref(obj);
	return(hd);
    }

    case SPLIT:{	/* Split list into two (roughly) equal halves */
	int l,x;
	struct object *hd = 0, **hdp = &hd, *top;

	if(
	    (obj->o_type != T_LIST) ||
	    ( (l = listlen(obj)) == 0 )
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
	l = ((l-1) >> 1)+1;
	for( x = 0, p = obj; x < l; ++x, p = CDR(p) ){
	    *hdp = q = obj_alloc(T_LIST);
	    hdp = &CDR(q);
	    CAR(q) = CAR(p);
	    CAR(p)->o_refs += 1;
	}
	CAR(top = obj_alloc(T_LIST)) = hd;
	hd = 0; hdp = &hd;
	while(p){
	    *hdp = q = obj_alloc(T_LIST);
	    hdp = &CDR(q);
	    CAR(q) = CAR(p);
	    CAR(p)->o_refs += 1;
	    p = CDR(p);
	}
	if( !hd ) hd = obj_alloc(T_LIST);
	CAR(CDR(top) = obj_alloc(T_LIST)) = hd;
	obj_unref(obj);
	return(top);
    }

    case ATOM:{
	int result;

	switch( obj->o_type ){
	case T_UNDEF:
	    return(obj);
	case T_INT:
	case T_BOOL:
	case T_FLOAT:
	    result = 1;
	    break;
	default:
	    result = 0;
	}
	p = obj_alloc(T_BOOL);
	p->o_val.o_int = result;
	obj_unref(obj);
	return(p);
    }

    case DIV:		/* Like '/', but forces integer operation */
	switch( numargs(obj) ){
	case T_UNDEF:
	    obj_unref(obj);
	    UNDEFINED();
	case T_FLOAT:
	case T_INT:{
	    int x1, x2;

	    x1 = NUMVAL(CAR(obj));
	    if( (x2 = NUMVAL(CAR(CDR(obj)))) == 0 ){
		obj_unref(obj);
		UNDEFINED();
	    }
	    p = obj_alloc(T_INT);
	    (p->o_val).o_int = x1 / x2;
	    obj_unref(obj);
	    return(p);
	}
	}
    

    case NIL:
	if( obj->o_type != T_LIST ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	p = obj_alloc(T_BOOL);
	if( CAR(obj) ) p->o_val.o_int = 0;
	else p->o_val.o_int = 1;
	obj_unref(obj);
	return(p);
    
    case EQ:
	return( eqobj(obj) );
    
    case AND:
	return( do_bool(obj,AND) );
    case OR:
	return( do_bool(obj,OR) );
    case XOR:
	return( do_bool(obj,XOR) );
    case NOT:
	if( obj->o_type != T_BOOL ){
	    obj_unref(obj);
	    UNDEFINED();
	}
	(p = obj_alloc(T_BOOL))->o_val.o_int = !obj->o_val.o_int;
	obj_unref(obj);
	return(p);
    
    default:
	fatal_err("Unrecognized symbol in do_intrinsics()");
    } /* Switch() */
    /*NOTREACHED*/
}

    /*
     * listlen()--return length of a list
     */
int
listlen(struct object *p)
{
    int l = 0;

    while( p && CAR(p) ){
	++l;
	p = CDR(p);
    }
    return(l);
}

    /*
     * Common code between distribute-left and -right
     */
static struct object *
do_dist(
        struct object *elem,
        struct object *lst,
        struct object *obj, /* Source object */
        int side)   /* Which side to stick on */
{
    struct object *r, *r2;
    struct object *hd, **hdp = &hd;

    if( !CAR(lst) ){		/* Distributing over NULL list */
	lst->o_refs += 1;
	obj_unref(obj);
	return(lst);
    }

	/*
	 * Evil C!  Line-by-line, here's what's happening
	 * 1. Get the first list element for the "lower" list
	 * 2. Bind the CAR of it to the distributing object,
	 *	incrementing that object's reference counter.
	 * 3. Get the second element for the "lower" list, bind
	 *	the CDR of the first element to it.
	 * 4. Bind the CAR of the second element to the current
	 *	element in the list being distributed over, increment
	 *	that object's reference count.
	 * 5. Allocate the "upper" list element, build it into the
	 *	chain.
	 * 6. Advance the chain building pointer to be ready to add
	 *	the next element.
	 * 7. Advance to next element of list being distributed over.
	 *
	 *  Gee, wasn't that easy?
	 */
    while( lst ){
	r = obj_alloc(T_LIST);
	if( !side ){
	    CAR(r) = elem;
	    elem->o_refs += 1;
	} else {
	    CAR(r) = CAR(lst);
	    CAR(lst)->o_refs += 1;
	}
	r2 = CDR(r) = obj_alloc(T_LIST);
	if( !side ){
	    CAR(r2) = CAR(lst);
	    CAR(lst)->o_refs += 1;
	} else {
	    CAR(r2) = elem;
	    elem->o_refs += 1;
	}
	*hdp = obj_alloc(T_LIST);
	CAR(*hdp) = r;
	hdp = &CDR(*hdp);

	lst = CDR(lst);
    }
    obj_unref(obj);
    return(hd);
}

    /*
     * do_trans()--transpose the elements of the "matrix"
     */
static struct object *
do_trans(struct object *obj)
{
    int len = 0, x, y;
    struct object *p, *q, *r;
    struct object *hd = 0, **hdp = &hd;

	/*
	 * Check argument, make sure first element is a list.
	 */
    if(
	( (p = obj)->o_type != T_LIST) ||
	!( p = CAR(obj) ) ||
	( p->o_type != T_LIST )
    ){
	obj_unref(obj);
	UNDEFINED();
    }

	/*
	 * Get how many down (len)
	 */
    len = listlen(p);

	/*
	 * Verify the structure.  Make sure each across is a list,
	 *	and of the same length.
	 */
    for( q = obj; q ; q = CDR(q) ){
	r = CAR(q);
	if(
	    (r->o_type != T_LIST) ||
	    (listlen(r) != len)
	){
	    obj_unref(obj);
	    UNDEFINED();
	}
    }

	/*
	 * By definition, list of NULL lists returns <>
	 */
    if( len == 0 ){
	obj_unref(obj);
	return( obj_alloc(T_LIST) );
    }

	/*
	 * Here is an O(n^3) way of building a transposed matrix.
	 *	Loop over each depth, building across.  I'm so debonnair
	 *	about it because I never use this blinking function.
	 */
    for( x = 0; x < len; ++x ){
	struct object *s = obj_alloc(T_LIST), *hd2 = 0, **hdp2 = &hd2;

	*hdp = s;
	hdp = &CDR(s);
	for( p = obj; p; p = CDR(p) ){
	    q = CAR(p);
	    for( y = 0; y < x; ++y )
		q = CDR(q);
	    q = CAR(q);
	    r = obj_alloc(T_LIST);
	    *hdp2 = r;
	    hdp2 = &CDR(r);
	    CAR(r) = q;
	    q->o_refs += 1;
	}
	CAR(s) = hd2;
    }
    obj_unref(obj);
    return(hd);
}

    /*
     * do_bool()--do the three boolean binary operators
     */
static struct object *
do_bool(struct object *obj, int op)
{
    struct object *p, *q;
    struct object *r;
    int i;

    if(
	(obj->o_type != T_LIST) ||
	( (p = CAR(obj))->o_type != T_BOOL) ||
	( (q = CAR(CDR(obj)))->o_type != T_BOOL)
    ){
	obj_unref(obj);
	UNDEFINED();
    }
    r = obj_alloc(T_BOOL);
    switch( op ){
    case AND:
	i = p->o_val.o_int && q->o_val.o_int;
	break;
    case OR:
	i = p->o_val.o_int || q->o_val.o_int;
	break;
    case XOR:
	i = (p->o_val.o_int || q->o_val.o_int) &&
	    !(p->o_val.o_int && q->o_val.o_int);
	break;
    default:
	fatal_err("Illegal binary logical op in do_bool()");
    }
    r->o_val.o_int = i;
    obj_unref(obj);
    return(r);
}
