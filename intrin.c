/*
 * intrin.c--intrinsic functions for FP.  These are the ones which
 *	parse as an identifier, and are symbol-tabled.
 *
 * 	Copyright (c) 1986 by Andy Valencia
 */
#include <math.h>
#include <stdio.h>
#include "fpcommon.h"
#include "intrin.h"
#include "misc.h"
#include "obj_type.hpp"
#include "pair_type.hpp"
#include "charfn.h"
#include "obj.h"
#include "object.hpp"
#include "yystype.h"
#include "symtab_entry.hpp"
#include "y.tab.h"

static live_obj_ptr
do_dist(live_obj_ptr elem, live_obj_ptr lst, live_obj_ptr obj, int side);

static live_obj_ptr do_trans(live_obj_ptr obj);
static live_obj_ptr do_bool(live_obj_ptr obj, int op);
static live_obj_ptr do_math_func(int tag, live_obj_ptr obj);

/// Pair up successive elements of a list
static live_obj_ptr do_pair(live_obj_ptr obj)
{
    if(
       (!obj->is_list()) ||
       !obj->car()
       ){
        obj_unref(obj);
        return undefined();
    }
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    obj_ptr q;
    obj_ptr r = nullptr;
    int x;
    obj_ptr p = obj;
    for(x = 0; p; p = p->cdr() ){
        if( x == 0 ){
            q = obj_alloc(nullptr);
            *hdp = q;
            hdp = q->cdr_addr();
            r = obj_alloc(nullptr);
            q->car(r);
            r->car(p->car());
            p->car()->inc_ref();
            x++;
        } else {
            q = obj_alloc(nullptr);
            r->cdr(q);
            q->car(p->car());
            p->car()->inc_ref();
            x = 0;
        }
    }
    obj_unref(obj);
    return(hd);
}

/// Split list into two (roughly) equal halves
static live_obj_ptr do_split(live_obj_ptr obj)
{
    int l;
    if(
       (!obj->is_list()) ||
       ( (l = obj->list_length()) == 0 )
       ){
        obj_unref(obj);
        return undefined();
    }
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
    l = ((l-1) >> 1)+1;
    int x;
    auto p = obj;
    obj_ptr q;
    for( x = 0; x < l; ++x, p = p->cdr() ){
        q = obj_alloc(nullptr);
        *hdp = q;
        hdp = q->cdr_addr();
        q->car(p->car());
        p->car()->inc_ref();
    }
    auto top = obj_alloc(hd);
    hd = nullptr;
    hdp = &hd;
    while(p){
        q = obj_alloc(nullptr);
        *hdp = q;
        hdp = q->cdr_addr();
        q->car(p->car());
        p->car()->inc_ref();
        p = p->cdr();
    }
    if( !hd ) hd = obj_alloc(nullptr);
    obj_ptr result = obj_alloc(nullptr);
    top->cdr(result);
    top->cadr(hd);
    obj_unref(obj);
    return(top);
}

static live_obj_ptr do_atom(live_obj_ptr obj)
{
    bool result;
    
    switch( obj->type() ){
        case obj_type::T_UNDEF:
            return(obj);
        case obj_type::T_INT:
        case obj_type::T_BOOL:
        case obj_type::T_FLOAT:
            result = true;
            break;
        case obj_type::T_LIST:
            result = false;
    }
    auto p = obj_alloc(result);
    obj_unref(obj);
    return(p);
}

/// Main intrinsic processing routine
live_obj_ptr
do_intrinsics(live_sym_ptr act, live_obj_ptr obj)
{
    assert(act);

	/*
	 * Switch off the tokenal value assigned by YACC.  Depending on the
	 *	sophistication of your C compiler, this can generate some
	 *	truly horrendous code.  Be prepared!  Perhaps it would be
	 *	better to store a pointer to a function in with the symbol
	 *	table...
	 */
    const int tag = act->sym_val.YYint;
    switch( tag ){

    case LENGTH:{	// Length of a list
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        int l = 0;
        for(obj_ptr p = obj; p && p->car(); p = p->cdr() ) l++;
        obj_unref(obj);
        auto p = obj_alloc(l);
        return(p);
    }

    case ID: {		// Identity
            return(obj);
    }
    case OUT: {		// Identity, but print debug line too
        printf("out: ");
        obj_prtree(obj);
        putchar('\n');
        return(obj);
    }
    
    case FIRST:
    case HD: {		// First elem of a list
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        if (!obj->car()) {
            return obj;
        }
        obj_ptr p = obj->car();
        p->inc_ref();
        obj_unref(obj);
        assert(p);
        auto result = static_cast<live_obj_ptr>(p);
        return(result);
    }

    case TL: {		// Remainder of list
        if( (!obj->is_list()) || !obj->car() ){
            obj_unref(obj);
            return undefined();
        }
        obj_ptr p = obj->cdr();
        if(!p){
            p = obj_alloc(nullptr);
        } else {
            p->inc_ref();
        }
        obj_unref(obj);
        assert(p);
        auto result = static_cast<live_obj_ptr>(p);
        return(result);
    }

    case IOTA:{		// Given arg N, generate <1..N>
        if( !obj->is_num() ){
            obj_unref(obj);
            return undefined();
        }
        int l = (obj->is_int()) ? obj->int_val() : static_cast<int>(obj->float_val());
        obj_unref(obj);
        if( l < 0 ) return undefined();
        if( l == 0 ) return( obj_alloc(nullptr) );
        obj_ptr hd;
        obj_ptr *hdp = &hd;
        for(int x = 1; x <= l; x++ ){
            auto q = obj_alloc(x);
            auto p = obj_alloc(nullptr);
            *hdp = p;
            p->car(q);
            hdp = p->cdr_addr();
        }
        assert(hd);
        auto result = static_cast<live_obj_ptr>(hd);
        return(result);
    } // Local block for IOTA

    case PICK:{		// Parameterized selection
        int x;
        obj_ptr p;
        obj_ptr q;

            // Verify all elements which we will use
        if(
            (!obj->is_list()) ||
            ( (p = obj->car())->type() != obj_type::T_INT ) ||
            !(q = obj->cdr()) ||
            ( (q = q->car())->type() != obj_type::T_LIST) ||
            ( (x = p->int_val()) == 0 )
        ){
            obj_unref(obj);
            return undefined();
        }

            // If x is negative, we are counting from the end
        if( x < 0 ){
            const int tmp = q->list_length();

            x += (tmp + 1);
            if( x < 1 ){
                obj_unref(obj);
                return undefined();
            }
        }

            // Loop along the list until our count is expired
        for( ; x > 1; --x ){
            if( !q )
                break;
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
        assert(q);
        auto result = static_cast<live_obj_ptr>(q);
        return(result);
    }

    case LAST: {		// Return last element of list
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        if( !obj->car() )
            return(obj);
        obj_ptr p;
        obj_ptr q = obj;
        while( (p = q->cdr()) )
            q = p;
        q = q->car();
        q->inc_ref();
        obj_unref(obj);
        assert(q);
        auto result = static_cast<live_obj_ptr>(q);
        return(result);
    }
    
    case FRONT:
    case TLR:{		// Return a list of all but list
        obj_ptr q = obj;
        if(
            (!q->is_list()) ||
            !obj->car()
        ){
            obj_unref(obj);
            return undefined();
        }
        obj_ptr hd = nullptr;
        obj_ptr *hdp = &hd;
        while( q->cdr() ){
            auto p = obj_alloc(q->car());
            *hdp = p;
            if( p->car() ){
                p->car()->inc_ref();
            }
            hdp = p->cdr_addr();
            q = q->cdr();
        }
        obj_unref(obj);
        if( !hd )
            return( obj_alloc(nullptr) );
        else {
            assert(hd);
            auto result = static_cast<live_obj_ptr>(hd);
            return(result);
        }
    }

    case DISTL: {		// Distribute from left-most element
        obj_ptr p;
        obj_ptr q;
        if(
            (!obj->is_list()) ||
            ( !(q = obj->car()) ) ||
            (!obj->cdr()) ||
            (!(p = obj->cadr()) ) ||
            (!p->is_list())
        ){
            obj_unref(obj);
            return undefined();
        }
        return( do_dist(q,p,obj,0) );
    }

    case DISTR:	{	// Distribute from left-most element
        obj_ptr p;
        obj_ptr q;
        if(
            (!obj->is_list()) ||
            ( !(q = obj->car()) ) ||
            (!obj->cdr()) ||
            (!(p = obj->cadr()) ) ||
            (!q->is_list())
        ){
            obj_unref(obj);
            return undefined();
        }
        return( do_dist(p,q,obj,1) );
    }
    
    case APNDL:{	// Append element from left
        obj_ptr p;
        obj_ptr q;

        if(
            (!obj->is_list()) ||
            ( !(q = obj->car()) ) ||
            (!obj->cdr()) ||
            (!(p = obj->cadr()) ) ||
            (!p->is_list())
        ){
            obj_unref(obj);
            return undefined();
        }
        q->inc_ref();
        if( !p->car() ){		// Null list?
            obj_unref(obj);
            auto result = obj_alloc(q);
            return(result);		// Just return element
        }
        p->inc_ref();
        auto result = obj_alloc(q, p);
        obj_unref(obj);
        return(result);
    }

    case APNDR:{	// Append element from right
        obj_ptr hd = nullptr;
        obj_ptr *hdp = &hd;
        obj_ptr p;
        obj_ptr q;
        obj_ptr r;

        if(
            (!obj->is_list()) ||
            ( !(q = obj->car()) ) ||
            (!obj->cdr()) ||
            (!(r = obj->cadr()) ) ||
            (!q->is_list())
        ){
            obj_unref(obj);
            return undefined();
        }
        r->inc_ref();
        if( !q->car() ){		// Empty list
            obj_unref(obj);
            auto result = obj_alloc(r);
            return(result);		// Just return elem
        }

            /*
             * Loop through list, building a new one.  We can't just reuse
             *	the old one because we're modifying its end.
             */
        while( q ){
            p = obj_alloc(nullptr);
            *hdp = p;
            q->car()->inc_ref();
            p->car(q->car());
            hdp = p->cdr_addr();
            q = q->cdr();
        }

            // Tack the element onto the end of the built list
        p = obj_alloc(r);
        *hdp = p;
        obj_unref(obj);
        assert(hd);
        auto result = static_cast<live_obj_ptr>(hd);
        return(result);
    }

    case TRANS:	{	// Transposition
        return( do_trans(obj) );
    }
    
    case REVERSE:{	// Reverse all elements of a list

        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        obj_ptr p;
        obj_ptr q;
        obj_ptr r;
        if( !obj->car() )
            return(obj);
        for( p = nullptr, q = obj; q; q = q->cdr() ){
            r = obj_alloc(nullptr);
            r->cdr(p);
            p = r;
            p->car(q->car());
            q->car()->inc_ref();
        }
        obj_unref(obj);
        assert(p);
        auto result = static_cast<live_obj_ptr>(p);
        return(result);
    }

    case ROTL:{		// Rotate left
            // Wanna list
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }

        obj_ptr p;
        obj_ptr q;
            // Need two elems, otherwise be ID function
        if(
            !(obj->car()) ||
            !(q = obj->cdr()) ||
            !(q->car())
        ){
            return(obj);
        }

        obj_ptr hd = nullptr;
        obj_ptr *hdp = &hd;
            // Loop, starting from second.  Build parallel list.
        for( /* q has obj->cdr() */ ; q; q = q->cdr() ){
            p = obj_alloc(nullptr);
            *hdp = p;
            hdp = p->cdr_addr();
            p->car(q->car());
            q->car()->inc_ref();
        }
        p = obj_alloc(obj->car());
        *hdp = p;
        obj->car()->inc_ref();
        obj_unref(obj);
        assert(hd);
        auto result = static_cast<live_obj_ptr>(hd);
        return(result);
    }

    case ROTR:{		// Rotate right
            // Wanna list
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }

        obj_ptr p;
        obj_ptr q;
            // Need two elems, otherwise be ID function
        if(
            !(obj->car()) ||
            !(q = obj->cdr()) ||
            !(q->car())
        ){
            return(obj);
        }

        obj_ptr hd = nullptr;
        obj_ptr *hdp = &hd;
            // Loop over list.  Stop one short of end.
        for( q = obj; q->cdr(); q = q->cdr() ){
            p = obj_alloc(nullptr);
            *hdp = p;
            hdp = p->cdr_addr();
            p->car(q->car());
            q->car()->inc_ref();
        }
        auto result = obj_alloc(q->car(), hd);
        q->car()->inc_ref();
        obj_unref(obj);
        return(result);
    }

    case CONCAT:{		// Concatenate several lists
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        if( !obj->car() ) return(obj);
        obj_ptr hd = nullptr;
        obj_ptr *hdp = &hd;
        for(obj_ptr p = obj; p; p = p->cdr() ){
            auto q = p->car();
            if( !q->is_list() ){
                obj_unref(obj);
                obj_unref(hd);
                return undefined();
            }
            if( !q->car() )
                continue;
            for( ; q; q = q->cdr() ){
                obj_ptr r;
                r = obj_alloc(nullptr);
                *hdp = r;
                hdp = r->cdr_addr();
                r->car(q->car());
                q->car()->inc_ref();
            }
        }
        obj_unref(obj);
        if( !hd )
            return(obj_alloc(nullptr));
        assert(hd);
        auto result = static_cast<live_obj_ptr>(hd);
        return(result);
    }

    case SIN:
    case COS:
    case TAN:
    case ASIN:
    case ACOS:
    case ATAN:
    case EXP:
    case LOG: {		// log() function
        return do_math_func(tag, obj);
    }
    
    case MOD: {		// Modulo
        switch( pairtype(obj) ){
        case pair_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case pair_type::T_FLOAT:
        case pair_type::T_INT:{
            const int x1 = static_cast<int>(obj->car()->num_val());
            const int x2 = static_cast<int>(obj->cadr()->num_val());
            if( x2 == 0 ){
                obj_unref(obj);
                return undefined();
            }
            auto p = obj_alloc(x1 % x2);
            obj_unref(obj);
            return(p);
        }
        }
    }
    
    case PAIR:{
        auto result = do_pair(obj);
        return result;
    }

    case SPLIT:{
        auto result = do_split(obj);
        return result;
    }

    case ATOM:{
        auto p = do_atom(obj);
        return(p);
    }

    case DIV: {		// Like '/', but forces integer operation
        switch( pairtype(obj) ){
            case pair_type::T_UNDEF:
                obj_unref(obj);
                return undefined();
            case pair_type::T_FLOAT:
            case pair_type::T_INT:{
                const int x1 = static_cast<int>(obj->car()->num_val());
                const int x2 = static_cast<int>(obj->cadr()->num_val());
                if( x2 == 0 ){
                    obj_unref(obj);
                    return undefined();
                }
                auto p = obj_alloc(x1 / x2);
                obj_unref(obj);
                return(p);
            }
        }
    }

    case NIL: {
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        bool value;
        if( obj->car() )
            value = false;
        else
            value = true;
        auto p = obj_alloc(value);
        obj_unref(obj);
        return(p);
    }
    
    case EQ: {
        return( eqobj(obj) );
    }
    
    case AND:
    case OR:
    case XOR: {
        return( do_bool(obj,tag) );
    }
    case NOT: {
        if( !obj->is_bool() ){
            obj_unref(obj);
            return undefined();
        }
        const auto value = !obj->bool_val();
        auto p = obj_alloc(value);
        obj_unref(obj);
        return(p);
    }
    
    default:
            fatal_err("Unrecognized symbol in do_intrinsics()");
    } // Switch()
}

/// Common code between distribute-left and -right
static live_obj_ptr
do_dist(
        live_obj_ptr elem,
        live_obj_ptr lst,
        live_obj_ptr obj, // Source object
        int side)   // Which side to stick on
{
    if( !lst->car() ){        // Distributing over NULL list
        lst->inc_ref();
        obj_unref(obj);
        return(lst);
    }

    /*
     * Evil C!  Line-by-line, here's what's happening
     * 1. Get the first list element for the "lower" list
     * 2. Bind the CAR of it to the distributing object,
     *    incrementing that object's reference counter.
     * 3. Get the second element for the "lower" list, bind
     *    the CDR of the first element to it.
     * 4. Bind the CAR of the second element to the current
     *    element in the list being distributed over, increment
     *    that object's reference count.
     * 5. Allocate the "upper" list element, build it into the
     *    chain.
     * 6. Advance the chain building pointer to be ready to add
     *    the next element.
     * 7. Advance to next element of list being distributed over.
     *
     *  Gee, wasn't that easy?
     */
    obj_ptr r;
    obj_ptr r2;
    obj_ptr hd;
    obj_ptr *hdp = &hd;
    while( lst ){
        r = obj_alloc(nullptr);
        if( !side ){
            r->car(elem);
            elem->inc_ref();
        } else {
            r->car(lst->car());
            lst->car()->inc_ref();
        }
        r->cdr(obj_alloc(nullptr));
        r2 = r->cdr();
        if( !side ){
            r2->car(lst->car());
            lst->car()->inc_ref();
        } else {
            r2->car(elem);
            elem->inc_ref();
        }
        *hdp = obj_alloc(nullptr);
        ((*hdp))->car(r);
        hdp = (*hdp)->cdr_addr();

        lst = lst->cdr();
    }
    obj_unref(obj);
    assert(hd);
    auto result = static_cast<live_obj_ptr>(hd);
    return(result);
}

/// do_trans()--transpose the elements of the "matrix"
static live_obj_ptr
do_trans(live_obj_ptr obj)
{
    obj_ptr p = obj;

    // Check argument, make sure first element is a list.
    if(
    ( !p->is_list()) ||
    !( p = obj->car() ) ||
    ( !p->is_list() )
    ){
        obj_unref(obj);
        return undefined();
    }

    obj_ptr q;
    obj_ptr hd = nullptr;
    obj_ptr *hdp = &hd;
	// Get how many down (len)
    int len = p->list_length();

	/*
	 * Verify the structure.  Make sure each across is a list,
	 *	and of the same length.
	 */
    for( q = obj; q ; q = q->cdr() ){
        auto r = q->car();
        if(
            (!r->is_list()) ||
            (r->list_length() != len)
        ){
            obj_unref(obj);
            return undefined();
        }
    }

	// By definition, list of NULL lists returns <>
    if( len == 0 ){
        obj_unref(obj);
        return( obj_alloc(nullptr) );
    }

	/*
	 * Here is an O(n^3) way of building a transposed matrix.
	 *	Loop over each depth, building across.  I'm so debonnair
	 *	about it because I never use this blinking function.
	 */
    for(int x = 0; x < len; ++x ){
        obj_ptr s = obj_alloc(nullptr);
        obj_ptr hd2 = nullptr;
        obj_ptr *hdp2 = &hd2;

        *hdp = s;
        hdp = s->cdr_addr();
        for( p = obj; p; p = p->cdr() ){
            q = p->car();
            for(int y = 0; y < x; ++y )
                q = q->cdr();
            q = q->car();
            auto r = obj_alloc(nullptr);
            *hdp2 = r;
            hdp2 = r->cdr_addr();
            r->car(q);
            q->inc_ref();
        }
        s->car(hd2);
    }
    obj_unref(obj);
    return(hd);
}

/// do_bool()--do the three boolean binary operators
static live_obj_ptr
do_bool(live_obj_ptr obj, int op)
{
    //TODO use is_pair()?
    if (!obj->is_list()) {
        obj_unref(obj);
        return undefined();
    }
    
    obj_ptr p = obj->car();
    obj_ptr q = obj->cadr();

    if(
    ( !p->is_bool()) ||
    ( !q->is_bool())
    ){
        obj_unref(obj);
        return undefined();
    }
    bool i;
    switch( op ){
        case AND:
            i = p->bool_val() && q->bool_val();
            break;
        case OR:
            i = p->bool_val() || q->bool_val();
            break;
        case XOR:
            i = (p->bool_val() || q->bool_val()) &&
                !(p->bool_val() && q->bool_val());
            break;
        default:
            fatal_err("Illegal binary logical op in do_bool()");
    }
    auto r = obj_alloc(i);
    obj_unref(obj);
    return(r);
}

static live_obj_ptr
do_math_func(int tag, live_obj_ptr obj)
{
    if( !obj->is_num() ){
        obj_unref(obj);
        return undefined();
    }
    const auto f = obj->num_val();
    double result;
    switch(tag) {
        case SIN: {        // sin() function
            result = sin(f);
            break;
        }
            
        case COS: {        // cos() function
            result = cos(f);
            break;
        }
            
        case TAN: {        // tan() function
            result = tan(f);
            break;
        }
            
        case ASIN: {        // asin() function
            result = asin(f);
            break;
        }
            
        case ACOS: {        // acos() function
            result = acos(f);
            break;
        }
            
        case ATAN: {        // atan() function
            result = atan(f);
            break;
        }
            
        case EXP: {        // exp() function
            result = exp(f);
            break;
        }
            
        case LOG: {        // log() function
            result = log(f);
            break;
        }
        default: {
            fatal_err("Unreachable case in do_trig");
        }
    }
    auto p = obj_alloc(result);
    obj_unref(obj);
    return(p);
}
