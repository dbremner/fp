/*
 * charfn.c--functions to do the "character" functions, like +, -, ...
 *
 * 	Copyright (c) 1986 by Andy Valencia
 */
#include "typedefs.h"
#include "fp.h"
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
    if( o1->o_type != o2->o_type ){
        if( o1->is_int() )
            if( o2->is_float() )
            return( o1->o_val.o_int == o2->o_val.o_double );
        if( o2->is_int() )
            if( o1->is_float() )
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
            return( same(o1->car(),o2->car()) && same(o1->cdr(),o2->cdr()) );
        case obj_type::T_UNDEF:
            fatal_err("Bad AST type in same()");
    }
}

/// ispair()--tell if our argument object is a list of two elements
static bool
ispair(obj_ptr obj)
{
    assert(obj);
    if( obj->o_type != obj_type::T_LIST )
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
obj_ptr
eqobj(obj_ptr obj)
{
    assert(obj);
    if( !ispair(obj) ){
        obj_unref(obj);
        return undefined();
    }
    obj_ptr p = obj_alloc(obj_type::T_BOOL);
    if( same(obj->car(),obj->cadr()) )
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
    assert(obj);
    obj_ptr p = eqobj(obj);

    if( p->o_type == obj_type::T_BOOL )
        p->o_val.o_int = (p->o_val.o_int ? 0 : 1);
    return(p);
}

/// do_charfun()--execute the action of a binary function
obj_ptr
do_charfun(ast_ptr act, obj_ptr obj)
{
    assert(act);
    assert(obj);
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
        case obj_type::T_INT: {
            bool result = obj->car()->num_val() > obj->cadr()->num_val();
            p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}

    case GE:
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            bool result = obj->car()->num_val() >= obj->cadr()->num_val();
            p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}

    case LE:
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            bool result = obj->car()->num_val() <= obj->cadr()->num_val();
            p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}

    case '<':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
        case obj_type::T_INT: {
            bool result = obj->car()->num_val() < obj->cadr()->num_val();
            p = obj_alloc(result);
            obj_unref(obj);
            return(p);
        }
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}

    case '+':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
            p = obj_alloc(obj_type::T_FLOAT);
            (p->o_val).o_double = obj->car()->num_val()+obj->cadr()->num_val();
            obj_unref(obj);
            return(p);
        case obj_type::T_INT:
            p = obj_alloc(obj_type::T_INT);
            (p->o_val).o_int = obj->car()->num_val()+obj->cadr()->num_val();
            obj_unref(obj);
            return(p);
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}
    case '-':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
            p = obj_alloc(obj_type::T_FLOAT);
            (p->o_val).o_double = obj->car()->num_val()-obj->cadr()->num_val();
            obj_unref(obj);
            return(p);
        case obj_type::T_INT:
            p = obj_alloc(obj_type::T_INT);
            (p->o_val).o_int = obj->car()->num_val()-obj->cadr()->num_val();
            obj_unref(obj);
            return(p);
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}
    case '*':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
            p = obj_alloc(obj_type::T_FLOAT);
            (p->o_val).o_double = obj->car()->num_val()*obj->cadr()->num_val();
            obj_unref(obj);
            return(p);
        case obj_type::T_INT:
            p = obj_alloc(obj_type::T_INT);
            (p->o_val).o_int = obj->car()->num_val()*obj->cadr()->num_val();
            obj_unref(obj);
            return(p);
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
	}
    case '/':
	switch( numargs(obj) ){
        case obj_type::T_UNDEF:
            obj_unref(obj);
            return undefined();
        case obj_type::T_FLOAT:
        case obj_type::T_INT:
            f = obj->cadr()->num_val();
            if( f == 0.0 ){
            obj_unref(obj);
            return undefined();
            }
            p = obj_alloc(obj->car()->num_val()/f);
            obj_unref(obj);
            return(p);
        case obj_type::T_LIST:
        case obj_type::T_BOOL:
            fatal_err("Unreachable switch cases");
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
