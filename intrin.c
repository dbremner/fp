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
     * CDR is like CAR but gives all but the first
     */

#define NUMVAL(x) ( (x->o_type == obj_type::T_INT) ? \
    ((x->o_val).o_int) : ((x->o_val).o_double) )
#define CDR(x) ( ((x)->o_val).o_list.cdr )

static obj_ptr
do_dist(obj_ptr elem, obj_ptr lst, obj_ptr obj, int side);

static obj_ptr do_trans(obj_ptr obj);
static obj_ptr do_bool(obj_ptr obj, int op);

/// Main intrinsic processing routine
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

    case LENGTH:{	// Length of a list
	int l;

	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	for( p = obj, l = 0; p && p->car(); p = p->cdr() ) l++;
	obj_unref(obj);
	p = obj_alloc(obj_type::T_INT);
	p->o_val.o_int = l;
	return(p);
    }

    case ID:		// Identity
	return(obj);
    case OUT:		// Identity, but print debug line too
	printf("out: ");
	obj_prtree(obj);
	putchar('\n');
	return(obj);
    
    case FIRST:
    case HD:		// First elem of a list
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj); return undefined();
	}
	if( !(p = obj->car()) ) return(obj);
	p->inc_ref();
	obj_unref(obj);
	return(p);

    case TL:		// Remainder of list
	if( (obj->o_type != obj_type::T_LIST) || !obj->car() ){
	    obj_unref(obj); return undefined();
	}
	if( !(p = obj->cdr()) ){
	    p = obj_alloc(obj_type::T_LIST);
	} else {
	    p->inc_ref();
	}
	obj_unref(obj);
	return(p);

    case IOTA:{		// Given arg N, generate <1..N>
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
	    p->car(q);
	    hdp = &CDR(p);
	}
	return(hd);
    } // Local block for IOTA

    case PICK:{		// Parameterized selection
	int x;

	    // Verify all elements which we will use
	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( (p = obj->car())->o_type != obj_type::T_INT ) ||
	    !(q = obj->cdr()) ||
	    ( (q = q->car())->o_type != obj_type::T_LIST) ||
	    ( (x = p->o_val.o_int) == 0 )
	){
	    obj_unref(obj);
	    return undefined();
	}

	    // If x is negative, we are counting from the end
	if( x < 0 ){
	    int tmp = listlen(q);

	    x += (tmp + 1);
	    if( x < 1 ){
		obj_unref(obj);
		return undefined();
	    }
	}

	    // Loop along the list until our count is expired
	for( ; x > 1; --x ){
	    if( !q ) break;
	    q = q->cdr();
	}

	    // If fell off the list, error
	if( !q || !(q = q->car()) ){
	    obj_unref(obj);
	    return undefined();
	}

	    // Add a reference to the named object, release the old object
	q->inc_ref();
	obj_unref(obj);
	return(q);
    }

    case LAST:		// Return last element of list
	if( (q = obj)->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !obj->car() ) return(obj);
    while( (p = q->cdr()) ) q = p;
	q = q->car();
	q->inc_ref();
	obj_unref(obj);
	return(q);
    
    case FRONT:
    case TLR:{		// Return a list of all but list
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	if(
	    ((q = obj)->o_type != obj_type::T_LIST) ||
	    !obj->car()
	){
	    obj_unref(obj);
	    return undefined();
	}
	while( q->cdr() ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
        p->car(q->car());
        if( p->car() ){
		p->car()->inc_ref();
	    }
	    hdp = &CDR(p);
	    q = q->cdr();
	}
	obj_unref(obj);
	if( !hd ) return( obj_alloc(obj_type::T_LIST) );
	else return(hd);
    }

    case DISTL:		// Distribute from left-most element
	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = obj->car()) ) ||
	    (!obj->cdr()) ||
	    (!(p = obj->cadr()) ) ||
	    (p->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	return( do_dist(q,p,obj,0) );

    case DISTR:		// Distribute from left-most element
	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = obj->car()) ) ||
	    (!obj->cdr()) ||
	    (!(p = obj->cadr()) ) ||
	    (q->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	return( do_dist(p,q,obj,1) );
    
    case APNDL:{	// Append element from left
	obj_ptr r;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = obj->car()) ) ||
	    (!obj->cdr()) ||
	    (!(p = obj->cadr()) ) ||
	    (p->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	q->inc_ref();
	if( !p->car() ){		// Null list?
	    obj_unref(obj);
	    p = obj_alloc(obj_type::T_LIST);
	    p->car(q);
	    return(p);		// Just return element
	}
	p->inc_ref();
	r = obj_alloc(obj_type::T_LIST);
    r->cdr(p);
	r->car(q);
	obj_unref(obj);
	return(r);
    }

    case APNDR:{	// Append element from right
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr r;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    ( !(q = obj->car()) ) ||
	    (!obj->cdr()) ||
	    (!(r = obj->cadr()) ) ||
	    (q->o_type != obj_type::T_LIST)
	){
	    obj_unref(obj);
	    return undefined();
	}
	r->inc_ref();
	if( !q->car() ){		// Empty list
	    obj_unref(obj);
	    p = obj_alloc(obj_type::T_LIST);
        p->car(r);
	    return(p);		// Just return elem
	}

	    /*
	     * Loop through list, building a new one.  We can't just reuse
	     *	the old one because we're modifying its end.
	     */
	while( q ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    q->car()->inc_ref();
	    p->car(q->car());
	    hdp = &CDR(p);
	    q = q->cdr();
	}

	    // Tack the element onto the end of the built list
	*hdp = p = obj_alloc(obj_type::T_LIST);
    p->car(r);
	obj_unref(obj);
	return(hd);
    }

    case TRANS:		// Transposition
	return( do_trans(obj) );
    
    case REVERSE:{	// Reverse all elements of a list
	obj_ptr r;

	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !obj->car() ) return(obj);
	for( p = nullptr, q = obj; q; q = q->cdr() ){
	    r = obj_alloc(obj_type::T_LIST);
        r->cdr(p);
	    p = r;
	    p->car(q->car());
	    q->car()->inc_ref();
	}
	obj_unref(obj);
	return(p);
    }

    case ROTL:{		// Rotate left
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	    // Wanna list
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}

	    // Need two elems, otherwise be ID function
	if(
	    !(obj->car()) ||
	    !(q = obj->cdr()) ||
	    !(q->car())
	){
	    return(obj);
	}

	    // Loop, starting from second.  Build parallel list.
	for( /* q has obj->cdr() */ ; q; q = q->cdr() ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(p);
	    p->car(q->car());
	    q->car()->inc_ref();
	}
	*hdp = p = obj_alloc(obj_type::T_LIST);
    p->car(obj->car());
	obj->car()->inc_ref();
	obj_unref(obj);
	return(hd);
    }

    case ROTR:{		// Rotate right
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	    // Wanna list
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}

	    // Need two elems, otherwise be ID function
	if(
	    !(obj->car()) ||
	    !(q = obj->cdr()) ||
	    !(q->car())
	){
	    return(obj);
	}

	    // Loop over list.  Stop one short of end.
	for( q = obj; q->cdr(); q = q->cdr() ){
	    *hdp = p = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(p);
	    p->car(q->car());
	    q->car()->inc_ref();
	}
	p = obj_alloc(obj_type::T_LIST);
	p->car(q->car());
	q->car()->inc_ref();
    p->cdr(hd);
	obj_unref(obj);
	return(p);
    }

    case CONCAT:{		// Concatenate several lists
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr r;

	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	if( !obj->car() ) return(obj);
	for( p = obj; p; p = p->cdr() ){
	    q = p->car();
	    if( q->o_type != obj_type::T_LIST ){
		obj_unref(obj);
		obj_unref(hd);
		return undefined();
	    }
	    if( !q->car() ) continue;
	    for( ; q; q = q->cdr() ){
		*hdp = r = obj_alloc(obj_type::T_LIST);
		hdp = &CDR(r);
        r->car(q->car());
		q->car()->inc_ref();
	    }
	}
	obj_unref(obj);
	if( !hd )
	    return(obj_alloc(obj_type::T_LIST));
	return(hd);
    }

    case SIN:		// sin() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = sin(f);
	obj_unref(obj);
	return(p);

    case COS:		// cos() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = cos(f);
	obj_unref(obj);
	return(p);

    case TAN:		// tan() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = tan(f);
	obj_unref(obj);
	return(p);

    case ASIN:		// asin() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = asin(f);
	obj_unref(obj);
	return(p);

    case ACOS:		// acos() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = acos(f);
	obj_unref(obj);
	return(p);

    case ATAN:		// atan() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = atan(f);
	obj_unref(obj);
	return(p);
    
    case EXP:		// exp() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = exp(f);
	obj_unref(obj);
	return(p);
    
    case LOG:		// log() function
	if( !obj->is_num() ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_FLOAT);
	f = obj->num_val();
	p->o_val.o_double = log(f);
	obj_unref(obj);
	return(p);
    
    case MOD:		// Modulo
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:{
	    int x1, x2;

	    x1 = NUMVAL(obj->car());
        x2 = NUMVAL(obj->cadr());
	    if( x2 == 0 ){
		obj_unref(obj);
		return undefined();
	    }
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = x1 % x2;
	    obj_unref(obj);
	    return(p);
	}
    case obj_type::T_LIST:
    case obj_type::T_BOOL:
        fatal_err("Unreachable switch cases");
	}
    
    case PAIR:{		// Pair up successive elements of a list
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr r;
	int x;

	if(
	    (obj->o_type != obj_type::T_LIST) ||
	    !obj->car()
	){
	    obj_unref(obj);
	    return undefined();
	}
	for( p = obj, x = 0; p; p = p->cdr() ){
	    if( x == 0 ){
		*hdp = q = obj_alloc(obj_type::T_LIST);
		hdp = &CDR(q);
        r = obj_alloc(obj_type::T_LIST);
        q->car(r);
        r->car(p->car());
		p->car()->inc_ref();
		x++;
	    } else {
        q = obj_alloc(obj_type::T_LIST);
        r->cdr(q);
		q->car(p->car());
		p->car()->inc_ref();
		x = 0;
	    }
	}
	obj_unref(obj);
	return(hd);
    }

    case SPLIT:{	// Split list into two (roughly) equal halves
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
	for( x = 0, p = obj; x < l; ++x, p = p->cdr() ){
	    *hdp = q = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(q);
	    q->car(p->car());
	    p->car()->inc_ref();
	}
    top = obj_alloc(obj_type::T_LIST);
    top->car(hd);
	hd = nullptr; hdp = &hd;
	while(p){
	    *hdp = q = obj_alloc(obj_type::T_LIST);
	    hdp = &CDR(q);
	    q->car(p->car());
	    p->car()->inc_ref();
	    p = p->cdr();
	}
	if( !hd ) hd = obj_alloc(obj_type::T_LIST);
    obj_ptr result = obj_alloc(obj_type::T_LIST);
    top->cdr(result);
    top->cdr()->car(hd);
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
    case obj_type::T_LIST:
	    result = 0;
	}
	p = obj_alloc(obj_type::T_BOOL);
	p->o_val.o_int = result;
	obj_unref(obj);
	return(p);
    }

    case DIV:		// Like '/', but forces integer operation
	switch( numargs(obj) ){
	case obj_type::T_UNDEF:
	    obj_unref(obj);
	    return undefined();
	case obj_type::T_FLOAT:
	case obj_type::T_INT:{
	    int x1, x2;

	    x1 = NUMVAL(obj->car());
        x2 = NUMVAL(obj->cadr());
	    if( x2 == 0 ){
		obj_unref(obj);
		return undefined();
	    }
	    p = obj_alloc(obj_type::T_INT);
	    (p->o_val).o_int = x1 / x2;
	    obj_unref(obj);
	    return(p);
    case obj_type::T_LIST:
    case obj_type::T_BOOL:
        fatal_err("Unreachable switch cases");
	}
	}
    

    case NIL:
	if( obj->o_type != obj_type::T_LIST ){
	    obj_unref(obj);
	    return undefined();
	}
	p = obj_alloc(obj_type::T_BOOL);
	if( obj->car() ) p->o_val.o_int = 0;
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
    } // Switch()
}

/// listlen()--return length of a list
int
listlen(obj_ptr p)
{
    int l = 0;

    while( p && p->car() ){
	++l;
	p = p->cdr();
    }
    return(l);
}

/// Common code between distribute-left and -right
static obj_ptr
do_dist(
        obj_ptr elem,
        obj_ptr lst,
        obj_ptr obj, // Source object
        int side)   // Which side to stick on
{
    obj_ptr r;
    obj_ptr r2;
    obj_ptr hd;
    obj_ptr *hdp = &hd;

    if( !lst->car() ){		// Distributing over NULL list
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
        r->car(elem);
	    elem->inc_ref();
	} else {
        r->car(lst->car());
	    lst->car()->inc_ref();
	}
    r->cdr(obj_alloc(obj_type::T_LIST));
	r2 = r->cdr();
	if( !side ){
        r2->car(lst->car());
	    lst->car()->inc_ref();
	} else {
        r2->car(elem);
	    elem->inc_ref();
	}
	*hdp = obj_alloc(obj_type::T_LIST);
    ((*hdp))->car(r);
	hdp = &CDR(*hdp);

	lst = lst->cdr();
    }
    obj_unref(obj);
    return(hd);
}

/// do_trans()--transpose the elements of the "matrix"
static obj_ptr
do_trans(obj_ptr obj)
{
    int len = 0, x, y;
    obj_ptr p;
    obj_ptr q;
    obj_ptr r;
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;

	// Check argument, make sure first element is a list.
    if(
	( (p = obj)->o_type != obj_type::T_LIST) ||
	!( p = obj->car() ) ||
	( p->o_type != obj_type::T_LIST )
    ){
	obj_unref(obj);
	return undefined();
    }

	// Get how many down (len)
    len = listlen(p);

	/*
	 * Verify the structure.  Make sure each across is a list,
	 *	and of the same length.
	 */
    for( q = obj; q ; q = q->cdr() ){
	r = q->car();
	if(
	    (r->o_type != obj_type::T_LIST) ||
	    (listlen(r) != len)
	){
	    obj_unref(obj);
	    return undefined();
	}
    }

	// By definition, list of NULL lists returns <>
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
	for( p = obj; p; p = p->cdr() ){
	    q = p->car();
	    for( y = 0; y < x; ++y )
		q = q->cdr();
	    q = q->car();
	    r = obj_alloc(obj_type::T_LIST);
	    *hdp2 = r;
	    hdp2 = &CDR(r);
	    r->car(q);
	    q->inc_ref();
	}
    s->car(hd2);
    }
    obj_unref(obj);
    return(hd);
}

/// do_bool()--do the three boolean binary operators
static obj_ptr
do_bool(obj_ptr obj, int op)
{
    obj_ptr p;
    obj_ptr q;
    obj_ptr r;
    int i;

    if(
	(obj->o_type != obj_type::T_LIST) ||
	( (p = obj->car())->o_type != obj_type::T_BOOL) ||
	( (q = obj->cadr())->o_type != obj_type::T_BOOL)
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
