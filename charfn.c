/*
 * charfn.c--functions to do the "character" functions, like +, -, ...
 *
 * 	Copyright (c) 1986 by Andy Valencia
 */
#include "fpcommon.h"
#include "list.h"
#include "yystype.h"
#include "ast.hpp"
#include "obj_type.hpp"
#include "charfn.h"
#include "misc.h"
#include "obj.h"
#include "object.hpp"
#include "y.tab.h"

    /*
     * same()--looks at two objects and tells whether they are the same.
     *	We recurse if it is a list.
     */
static bool
same(obj_ptr o1, obj_ptr o2)
{
    if( o1 == o2 )
        return(true);
    assert(o1);
    assert(o2);
    if( o1->type() != o2->type() ){
        if( o1->is_int() )
            if( o2->is_float() )
            return( o1->int_val() == o2->o_val.o_double );
        if( o2->is_int() )
            if( o1->is_float() )
            return( o2->int_val() == o1->o_val.o_double );
        return(false);
    }
    switch( o1->type() ){
        case obj_type::T_INT:
        case obj_type::T_BOOL:
            return( o1->o_val.o_int == o2->o_val.o_int );
        case obj_type::T_FLOAT:
            return( o1->o_val.o_double == o2->o_val.o_double );
        case obj_type::T_LIST:
            return( same(o1->car(),o2->car()) && same(o1->cdr(),o2->cdr()) );
        case obj_type::T_UNDEF:
            fatal_err("Bad AST type in same()");
    }
}

/// ispair()--tell if our argument object is a list of two elements
static bool
ispair(live_obj_ptr obj)
{
    assert(obj);
    if( obj->type() != obj_type::T_LIST )
        return(false);
    if( obj->car() == nullptr )
        return(false);
    if( obj->cdr() == nullptr )
        return(false);
    if( obj->cdr()->cdr() )
        return(false);
    return(true);
}

    /*
     * eqobj()--tell if the two objects in the list are equal.
     *	undefined on ill-formed list, etc.
     */
live_obj_ptr
eqobj(live_obj_ptr obj)
{
    assert(obj);
    if( !ispair(obj) ){
        obj_unref(obj);
        return undefined();
    }
    const bool result = same(obj->car(),obj->cadr());
    auto p = obj_alloc(result);
    obj_unref(obj);
    return(p);
}

/// noteqobj()--just like eqobj(), but not equal
static live_obj_ptr
noteqobj(live_obj_ptr obj)
{
    assert(obj);
    auto p = eqobj(obj);

    if( p->type() == obj_type::T_BOOL )
        p->o_val.o_int = (p->o_val.o_int ? 0 : 1);
    return(p);
}

/// do_charfun()--execute the action of a binary function
live_obj_ptr
do_charfun(live_ast_ptr act, live_obj_ptr obj)
{
    assert(act);
    assert(obj);

    switch( (act->val).YYint ){

    case '=': {
        return( eqobj(obj) );
    }
    case NE: {
        return( noteqobj(obj) );
    }

    case '>':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            const bool result = obj->car()->num_val() > obj->cadr()->num_val();
            auto p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}

    case GE:
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            const bool result = obj->car()->num_val() >= obj->cadr()->num_val();
            auto p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}

    case LE:
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            const bool result = obj->car()->num_val() <= obj->cadr()->num_val();
            auto p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}

    case '<':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            const bool result = obj->car()->num_val() < obj->cadr()->num_val();
            auto p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}

    case '+':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT: {
            const auto value = obj->car()->num_val()+obj->cadr()->num_val();
            auto p = obj_alloc(value);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_INT: {
            const int value = obj->car()->num_val()+obj->cadr()->num_val();
            auto p = obj_alloc(value);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}
    case '-':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT: {
            const auto value = obj->car()->num_val()-obj->cadr()->num_val();
            auto p = obj_alloc(value);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_INT: {
            const int value = obj->car()->num_val()-obj->cadr()->num_val();
            auto p = obj_alloc(value);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}
    case '*':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT: {
            const auto value = obj->car()->num_val()*obj->cadr()->num_val();
            auto p = obj_alloc(value);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_INT: {
            const int value = obj->car()->num_val()*obj->cadr()->num_val();
            auto p = obj_alloc(value);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
	}
    case '/':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF: {
            obj_unref(obj);
            return undefined();
        }
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            const auto f = obj->cadr()->num_val();
            if( f == 0.0 ){
            obj_unref(obj);
            return undefined();
            }
            auto p = obj_alloc(obj->car()->num_val()/f);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL: {
            fatal_err("Unreachable switch cases");
        }
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
numargs(live_obj_ptr obj)
{
    assert(obj);
    // Don't have a well-formed list, so illegal
    if( !ispair(obj) ) return(obj_type::T_UNDEF);

	/*
	 * So it's a list of two.  Verify type of both elements.
	 *	'p' gets the first object, 'q' gets second.
	 */
    obj_ptr p = obj->car();
    obj_ptr q = obj->cadr();
    if( !p->is_num() || !q->is_num() ) return(obj_type::T_UNDEF);
    if( (p->is_float()) || (q->is_float()) )
	return(obj_type::T_FLOAT);
    return(obj_type::T_INT);
}
