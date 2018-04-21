/*
 * intrin.c--intrinsic functions for FP.  These are the ones which
 *	parse as an identifier, and are symbol-tabled.
 *
 * 	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"
#include <math.h>
#include <stdio.h>

    /*
     * This ugly set of macros makes access to objects easier.
     *
     * NUMVAL generates a value for C of the correct type
     * CAR manipulates the object as a list & gives its first part
     * CDR is like CAR but gives all but the first
     */

#define NUMVAL(x) ( (x->o_type == obj_type::T_INT) ? \
    ((x->o_val).o_int) : ((x->o_val).o_double) )
#define CAR(x) ( ((x)->o_val).o_list.car )
#define CDR(x) ( ((x)->o_val).o_list.cdr )

static obj_ptr
do_dist(obj_ptr elem, obj_ptr lst, obj_ptr obj, int side);

static obj_ptr do_trans(obj_ptr obj);
static obj_ptr do_bool(obj_ptr obj, int op);

    /*
     * Main intrinsic processing routine
     */
obj_ptr
do_intrinsics(sym_ptr act, obj_ptr obj)
{
    obj_ptr p;
    obj_ptr q;
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

	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	for( p = obj, l = 0; p && car_(p); p = cdr_(p) ) l++;
	obj_unref(obj);
	p = obj_alloc(obj_type::T_INT);
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
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj); return undefined();
	}
	if( !(p = car_(obj)) ) return(obj);
	p->inc_ref();
	obj_unref(obj);
	return(p);

    case TL:		/* Remainder of list */
	if( (obj->o_type != obj_type::T_LIST) || !car_(obj) ){
	    obj_unref(obj); return undefined();
	}
	if( !(p = cdr_(obj)) ){
	    p = obj_alloc(obj_type::T_LIST);
	} else {
	    p->inc_ref();
	}
	obj_unref(obj);
	return(p);

    case IOTA:{		/* Given arg N, generate <1..N> */
	int x, l;
    obj_ptr hd;
    obj_ptr *hdp = &hd;

	if( (obj->o_type != obj_type::T_INT) && (obj->o_type != obj_type::T_FLOAT) ){
	    obj_unref(obj);
	    return undefined();
	}
	l = (obj->o_type == obj_type::T_INT) ? obj->o_val.o_int : obj->o_val.o_double;
	obj_unref(obj);
	if( l < 0 ) return undefined();
	if( l == 0 ) return( obj_alloc(obj_type::T_LIST) );
	for( x = 1; x <= l; x++ ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    q = obj_alloc(obj_type::T_INT);
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
	    (obj->o_type != obj_type::T_LIST) ||
	    ( (p = car_(obj))->o_type != obj_type::T_INT ) ||
	    !(q = cdr_(obj)) ||
	    ( (q = car_(q))->o_type != obj_type::T_LIST) ||
	    ( (x = p->o_val.o_int) == 0 )
	){
	    obj_unref(obj);
	    return undefined();
	}

	    /*
	     * If x is negative, we are counting from the end
	     */
	if( x < 0 ){
	    int tmp = listlen(q);

	    x += (tmp + 1);
	    if( x < 1 ){
		obj_unref(obj);
		return undefined();
	    }
	}

	    /*
	     * Loop along the list until our count is expired
	     */
	for( ; x > 1; --x ){
	    if( !q ) break;
	    q = cdr_(q);
	}

	    /*
	     * If fell off the list, error
	     */
	if( !q || !(q = car_(q)) ){
	    obj_unref(obj);
	    return undefined();
	}

	    /*
	     * Add a reference to the named object, release the old object
	     */
	q->inc_ref();
	obj_unref(obj);
	return(q);
    }

    case LAST:		/* Return last element of list */
	if( (q = obj)->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !car_(obj) ) return(obj);
    while( (p = cdr_(q)) ) q = p;
	q = car_(q);
	q->inc_ref();
	obj_unref(obj);
	return(q);
    
    case FRONT:
    case TLR:{		/* Return a list of all but list */
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	if(
	    ((q = obj)->o_type != obj_type::T_LIST) ||
	    !car_(obj)
	){
	    obj_unref(obj);
	    return undefined();
	}
	while( cdr_(q) ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
        if( (CAR(p) = car_(q)) ){
		car_(p)->inc_ref();
	    }
	    hdp = &CDR(p);
	    q = cdr_(q);
	}
	obj_unref(obj);
	if( !hd ) return( obj_alloc(obj_type::T_LIST) );
	else return(hd);
    }

    case DISTL:		/* Distribute from left-most element */
	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = car_(obj)) ) ||
	    (!cdr_(obj)) ||
	    (!(p = car_(cdr_(obj))) ) ||
	    (p->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	return( do_dist(q,p,obj,0) );

    case DISTR:		/* Distribute from left-most element */
	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = car_(obj)) ) ||
	    (!cdr_(obj)) ||
	    (!(p = car_(cdr_(obj))) ) ||
	    (q->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	return( do_dist(p,q,obj,1) );
    
    case APNDL:{	/* Append element from left */
	obj_ptr r;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = car_(obj)) ) ||
	    (!cdr_(obj)) ||
	    (!(p = car_(cdr_(obj))) ) ||
	    (p->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	q->inc_ref();
	if( !car_(p) ){		/* Null list? */
	    obj_unref(obj);
	    p = obj_alloc(obj_type::T_LIST);
	    CAR(p) = q;
	    return(p);		/* Just return element */
	}
	p->inc_ref();
	r = obj_alloc(obj_type::T_LIST);
	CDR(r) = p;
	CAR(r) = q;
	obj_unref(obj);
	return(r);
    }

    case APNDR:{	/* Append element from right */
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr r;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = car_(obj)) ) ||
	    (!cdr_(obj)) ||
	    (!(r = car_(cdr_(obj))) ) ||
	    (q->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	r->inc_ref();
	if( !car_(q) ){		/* Empty list */
	    obj_unref(obj);
	    p = obj_alloc(obj_type::T_LIST);
	    CAR(p) = r;
	    return(p);		/* Just return elem */
	}

	    /*
	     * Loop through list, building a new one.  We can't just reuse
	     *	the old one because we're modifying its end.
	     */
	while( q ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    car_(q)->inc_ref();
	    CAR(p) = car_(q);
	    hdp = &CDR(p);
	    q = CDR(q);
	}

	    /*
	     * Tack the element onto the end of the built list
	     */
	*hdp = p = obj_alloc(obj_type::T_LIST);
	CAR(p) = r;
	obj_unref(obj);
	return(hd);
    }

    case TRANS:		/* Transposition */
	return( do_trans(obj) );
    
    case REVERSE:{	/* Reverse all elements of a list */
	obj_ptr r;

	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !car_(obj) ) return(obj);
	for( p = nullptr, q = obj; q; q = cdr_(q) ){
	    r = obj_alloc(obj_type::T_LIST);
	    CDR(r) = p;
	    p = r;
	    CAR(p) = car_(q);
	    car_(q)->inc_ref();
	}
	obj_unref(obj);
	return(p);
    }

    case ROTL:{		/* Rotate left */
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	    /*
	     * Wanna list
	     */
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}

	    /*
	     * Need two elems, otherwise be ID function
	     */
	if(
	    !(car_(obj)) ||
	    !(q = cdr_(obj)) ||
	    !(car_(q))
	){
	    return(obj);
	}

	    /*
	     * Loop, starting from second.  Build parallel list.
	     */
	for( /* q has cdr_(obj) */ ; q; q = cdr_(q) ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(p);
	    CAR(p) = car_(q);
	    car_(q)->inc_ref();
	}
	*hdp = p = obj_alloc(obj_type::T_LIST);
	CAR(p) = car_(obj);
	car_(obj)->inc_ref();
	obj_unref(obj);
	return(hd);
    }

    case ROTR:{		/* Rotate right */
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	    /*
	     * Wanna list
	     */
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}

	    /*
	     * Need two elems, otherwise be ID function
	     */
	if(
	    !(car_(obj)) ||
	    !(q = cdr_(obj)) ||
	    !(car_(q))
	){
	    return(obj);
	}

	    /*
	     * Loop over list.  Stop one short of end.
	     */
	for( q = obj; cdr_(q); q = cdr_(q) ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(p);
	    CAR(p) = car_(q);
	    car_(q)->inc_ref();
	}
	p = obj_alloc(obj_type::T_LIST);
	CAR(p) = car_(q);
	car_(q)->inc_ref();
	CDR(p) = hd;
	obj_unref(obj);
	return(p);
    }

    case CONCAT:{		/* Concatenate several lists */
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr r;

	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !car_(obj) ) return(obj);
	for( p = obj; p; p = cdr_(p) ){
	    q = car_(p);
	    if( q->o_type != obj_type::T_LIST ){
		obj_unref(obj);
		obj_unref(hd);
		return undefined();
	    }
	    if( !car_(q) ) continue;
	    for( ; q; q = cdr_(q) ){
		*hdp = r = obj_alloc(obj_type::T_LIST);
		hdp = &CDR(r);
		CAR(r) = car_(q);
		car_(q)->inc_ref();
	    }
	}
	obj_unref(obj);
	if( !hd )
	    return(obj_alloc(obj_type::T_LIST));
	return(hd);
    }

    case SIN:		/* sin() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = sin(f);
	obj_unref(obj);
	return(p);

    case COS:		/* cos() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = cos(f);
	obj_unref(obj);
	return(p);

    case TAN:		/* tan() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = tan(f);
	obj_unref(obj);
	return(p);

    case ASIN:		/* asin() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = asin(f);
	obj_unref(obj);
	return(p);

    case ACOS:		/* acos() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = acos(f);
	obj_unref(obj);
	return(p);

    case ATAN:		/* atan() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = atan(f);
	obj_unref(obj);
	return(p);
    
    case EXP:		/* exp() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = exp(f);
	obj_unref(obj);
	return(p);
    
    case LOG:		/* log() function */
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = NUMVAL(obj);
	p->o_val.o_double = log(f);
	obj_unref(obj);
	return(p);
    
    case MOD:		/* Modulo */
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:{
	    int x1, x2;

	    x1 = NUMVAL(car_(obj));
	    if( (x2 = NUMVAL(car_(cdr_(obj)))) == 0 ){
		obj_unref(obj);
		return undefined();
	    }
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = x1 % x2;
	    obj_unref(obj);
	    return(p);
	}
	}
    
    case PAIR:{		/* Pair up successive elements of a list */
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr r;
	int x;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    !car_(obj)
	){
	    obj_unref(obj);
	    return undefined();
	}
	for( p = obj, x = 0; p; p = cdr_(p) ){
	    if( x == 0 ){
		*hdp = q = obj_alloc(obj_type::T_LIST);
		hdp = &CDR(q);
		CAR(q) = r = obj_alloc(obj_type::T_LIST);
		CAR(r) = car_(p);
		car_(p)->inc_ref();
		x++;
	    } else {
		CDR(r) = q = obj_alloc(obj_type::T_LIST);
		CAR(q) = car_(p);
		car_(p)->inc_ref();
		x = 0;
	    }
	}
	obj_unref(obj);
	return(hd);
    }

    case SPLIT:{	/* Split list into two (roughly) equal halves */
	int l,x;
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr top;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( (l = listlen(obj)) == 0 )
	){
	    obj_unref(obj);
	    return undefined();
	}
	l = ((l-1) >> 1)+1;
	for( x = 0, p = obj; x < l; ++x, p = cdr_(p) ){
	    *hdp = q = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(q);
	    CAR(q) = car_(p);
	    car_(p)->inc_ref();
	}
	CAR(top = obj_alloc(obj_type::T_LIST)) = hd;
	hd = nullptr; hdp = &hd;
	while(p){
	    *hdp = q = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(q);
	    CAR(q) = car_(p);
	    car_(p)->inc_ref();
	    p = cdr_(p);
	}
	if( !hd ) hd = obj_alloc(obj_type::T_LIST);
	CAR(CDR(top) = obj_alloc(obj_type::T_LIST)) = hd;
	obj_unref(obj);
	return(top);
    }

    case ATOM:{
	int result;

	switch( obj->o_type ){
	case obj_type::T_UNDEF:
	    return(obj);
	case obj_type::T_INT:
	case obj_type::T_BOOL:
	case obj_type::T_FLOAT:
	    result = 1;
	    break;
	default:
	    result = 0;
	}
	p = obj_alloc(obj_type::T_BOOL);
	p->o_val.o_int = result;
	obj_unref(obj);
	return(p);
    }

    case DIV:		/* Like '/', but forces integer operation */
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:{
	    int x1, x2;

	    x1 = NUMVAL(car_(obj));
	    if( (x2 = NUMVAL(car_(cdr_(obj)))) == 0 ){
		obj_unref(obj);
		return undefined();
	    }
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = x1 / x2;
	    obj_unref(obj);
	    return(p);
	}
	}
    

    case NIL:
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_BOOL);
	if( car_(obj) ) p->o_val.o_int = 0;
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
	if( obj->o_type != obj_type::T_BOOL ){
	    obj_unref(obj);
	    return undefined();
	}
	(p = obj_alloc(obj_type::T_BOOL))->o_val.o_int = !obj->o_val.o_int;
	obj_unref(obj);
	return(p);
    
    default:
	fatal_err("Unrecognized symbol in do_intrinsics()");
    } /* Switch() */
}

    /*
     * listlen()--return length of a list
     */
int
listlen(obj_ptr p)
{
    int l = 0;

    while( p && car_(p) ){
	++l;
	p = cdr_(p);
    }
    return(l);
}

    /*
     * Common code between distribute-left and -right
     */
static obj_ptr
do_dist(
        obj_ptr elem,
        obj_ptr lst,
        obj_ptr obj, /* Source object */
        int side)   /* Which side to stick on */
{
    obj_ptr r;
    obj_ptr r2;
    obj_ptr hd;
    obj_ptr *hdp = &hd;

    if( !car_(lst) ){		/* Distributing over NULL list */
	lst->inc_ref();
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
	r = obj_alloc(obj_type::T_LIST);
	if( !side ){
	    CAR(r) = elem;
	    elem->inc_ref();
	} else {
	    CAR(r) = car_(lst);
	    car_(lst)->inc_ref();
	}
	r2 = CDR(r) = obj_alloc(obj_type::T_LIST);
	if( !side ){
	    CAR(r2) = car_(lst);
	    car_(lst)->inc_ref();
	} else {
	    CAR(r2) = elem;
	    elem->inc_ref();
	}
	*hdp = obj_alloc(obj_type::T_LIST);
	CAR(*hdp) = r;
	hdp = &CDR(*hdp);

	lst = cdr_(lst);
    }
    obj_unref(obj);
    return(hd);
}

    /*
     * do_trans()--transpose the elements of the "matrix"
     */
static obj_ptr
do_trans(obj_ptr obj)
{
    int len = 0, x, y;
    obj_ptr p;
    obj_ptr q;
    obj_ptr r;
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	/*
	 * Check argument, make sure first element is a list.
	 */
    if(
	( (p = obj)->o_type != obj_type::T_LIST) ||
	!( p = car_(obj) ) ||
	( p->o_type != obj_type::T_LIST )
    ){
	obj_unref(obj);
	return undefined();
    }

	/*
	 * Get how many down (len)
	 */
    len = listlen(p);

	/*
	 * Verify the structure.  Make sure each across is a list,
	 *	and of the same length.
	 */
    for( q = obj; q ; q = cdr_(q) ){
	r = car_(q);
	if(
	    (r->o_type != obj_type::T_LIST) ||
	    (listlen(r) != len)
	){
	    obj_unref(obj);
	    return undefined();
	}
    }

	/*
	 * By definition, list of NULL lists returns <>
	 */
    if( len == 0 ){
	obj_unref(obj);
	return( obj_alloc(obj_type::T_LIST) );
    }

	/*
	 * Here is an O(n^3) way of building a transposed matrix.
	 *	Loop over each depth, building across.  I'm so debonnair
	 *	about it because I never use this blinking function.
	 */
    for( x = 0; x < len; ++x ){
    obj_ptr s = obj_alloc(obj_type::T_LIST);
    obj_ptr hd2 = nullptr;
    obj_ptr *hdp2 = &hd2;

	*hdp = s;
	hdp = &CDR(s);
	for( p = obj; p; p = cdr_(p) ){
	    q = car_(p);
	    for( y = 0; y < x; ++y )
		q = cdr_(q);
	    q = car_(q);
	    r = obj_alloc(obj_type::T_LIST);
	    *hdp2 = r;
	    hdp2 = &CDR(r);
	    CAR(r) = q;
	    q->inc_ref();
	}
	CAR(s) = hd2;
    }
    obj_unref(obj);
    return(hd);
}

    /*
     * do_bool()--do the three boolean binary operators
     */
static obj_ptr
do_bool(obj_ptr obj, int op)
{
    obj_ptr p;
    obj_ptr q;
    obj_ptr r;
    int i;

    if(
	(obj->o_type != obj_type::T_LIST) ||
	( (p = car_(obj))->o_type != obj_type::T_BOOL) ||
	( (q = car_(cdr_(obj)))->o_type != obj_type::T_BOOL)
    ){
	obj_unref(obj);
	return undefined();
    }
    r = obj_alloc(obj_type::T_BOOL);
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
