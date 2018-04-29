/*
 * Execution module for FP.  Runs along the AST and executes actions.
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fpcommon.h"
#include "exec.h"
#include "yystype.h"
#include "ast.hpp"
#include "intrin.h"
#include "misc.h"
#include "obj_type.hpp"
#include "pair_type.hpp"
#include "charfn.h"
#include "obj.h"
#include "object.hpp"
#include "symtab_entry.hpp"
#include "y.tab.h"

static live_obj_ptr invoke(live_sym_ptr def, live_obj_ptr obj);
static live_obj_ptr do_rinsert(live_ast_ptr act, live_obj_ptr obj);
static live_obj_ptr do_binsert(live_ast_ptr act, live_obj_ptr obj);

    /*
     * Given an AST for an action, and an object to do the action upon,
     *	execute the action and return the result.
     */
live_obj_ptr
execute(live_ast_ptr act, live_obj_ptr obj )
{
    assert(act);

	// Broad categories of executable entities
    switch( act->tag ){

	// Invoke a user-defined function
    case 'U': {
        assert(act->val.YYsym);
        return( invoke(act->val.YYsym, obj) );
    }

	// Right-insert operator
    case '!': {
        assert(act->left);
        auto left = static_cast<live_ast_ptr>(act->left);
        return( do_rinsert(left, obj) );
    }

	// Binary-insert operator
    case '|': {
        assert(act->left);
        auto left = static_cast<live_ast_ptr>(act->left);
        return( do_binsert(left, obj) );
    }

	// Intrinsics
    case 'i': {
        assert(act->val.YYsym);
        return( do_intrinsics(act->val.YYsym, obj) );
    }

	// Select one element from a list
    case 'S': {
        if(
            (!obj->is_list()) ||
            !obj->car()
        ){
            obj_unref(obj);
            return undefined();
        }
        auto p = obj;
        int x = act->val.YYint;
        if( x == 0 ){
            obj_unref(obj);
            return undefined();
        }

            // Negative selectors count from end of list
        if( x < 0 ){
            const int tmp = p->list_length();

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
    }

	/*
	 * Apply the action on the left to the result of executing
	 *	the action on the right against the object.
	 */
    case '@': {
        assert(act->left);
        assert(act->right);
        auto left = static_cast<live_ast_ptr>(act->left);
        auto right = static_cast<live_ast_ptr>(act->right);
        auto p = execute(right, obj );
        return( execute(left, p ) );
    }

	/*
	 * Build a new list by applying the listed actions to the object
	 *	All is complicated by the fact that we must be clean in
	 *	the presence of T_UNDEF popping up along the way.
	 */
    case '[':{
        assert(act->left);
        act = static_cast<live_ast_ptr>(act->left);
        obj_ptr hd = nullptr;
        obj_ptr *hdp = &hd;
        while( act ){
            obj->inc_ref();
            assert(act->left);
            auto left = static_cast<live_ast_ptr>(act->left);
            auto p = execute(left,obj);
            if( p->is_undef() ){
            obj_unref(hd);
            obj_unref(obj);
            return(p);
            }
            auto q = obj_alloc(p);
            *hdp = q;
            hdp = q->cdr_addr();
            act = act->right;
        }
        obj_unref(obj);
        assert(hd);
        auto result = static_cast<live_obj_ptr>(hd);
        return(result);
    }

	// These are the single-character operations (+, -, etc.)
    case 'c': {
        return(do_charfun(act,obj));
    }

	// Conditional.  Evaluate & return one of the two paths
    case '>': {
        obj->inc_ref();
        auto p = execute(act->left,obj);
        if( p->is_undef() ){
            obj_unref(obj);
            return(p);
        }
        if( !p->is_bool() ){
            obj_unref(obj);
            obj_unref(p);
            return undefined();
        }
        live_obj_ptr q;
        if( p->bool_val() ) {
            q = execute(act->middle,obj);
        }
        else {
            q = execute(act->right,obj);
        }
        obj_unref(p);
        return(q);
    }

	// Apply the action to each member of a list
    case '&': {
        if( !obj->is_list() ){
            obj_unref(obj);
            return undefined();
        }
        if( !obj->car() ) return(obj);
        obj_ptr hd = nullptr;
        auto hdp = &hd;
        for(auto p = obj; p; p = p->cdr() ){
            (p->car())->inc_ref();
            assert(act->left);
            auto left = static_cast<live_ast_ptr>(act->left);
            assert(p->car());
            auto p_car = static_cast<live_obj_ptr>(p->car());
            auto q = execute(left, p_car);
            if( q->is_undef() ){
            obj_unref(hd);
            obj_unref(obj);
            return(q);
            }
            auto r = obj_alloc(q);
            *hdp = r;
            hdp = r->cdr_addr();
        }
        obj_unref(obj);
        assert(hd);
        auto result = static_cast<live_obj_ptr>(hd);
        return(result);
    }

	// Introduce an object
    case '%': {
        if( obj->is_undef() ) return(obj);
        obj_unref(obj);
        auto p = act->val.YYobj;
        p->inc_ref();
        return(p);
    }
    
	// Do a while loop
    case 'W': {
        while( 1 ){
            if( obj->is_undef() ){
                break;
            }
            obj->inc_ref();
            assert(act->left);
            auto left = static_cast<live_ast_ptr>(act->left);
            auto p = execute(left,obj);
            if( !p->is_bool() ){
                obj_unref(p);
                break;
            }
            if( p->bool_val() ){
                obj_unref(p);
                assert(act->right);
                auto right = static_cast<live_ast_ptr>(act->right);
                obj = execute(right,obj);
            } else {
                obj_unref(p);
                return(obj);
            }
        }
        obj_unref(obj);
        return undefined();
    }

    default:
	fatal_err("Undefined AST tag in execute()");
    }
}

/// Call a previously-defined user function, or error
static live_obj_ptr
invoke(live_sym_ptr def, live_obj_ptr obj)
{
    // Must be a defined function
    if( def->sym_type != symtype::SYM_DEF ){
        printf("%s: undefined\n",def->sym_pname.c_str());
        obj_unref(obj);
        return( undefined() );
    }
    
    // Call it with the object
    return( execute( def->sym_val.YYast, obj ) );
}

/// Local function to handle the tedious right-inserting
static live_obj_ptr
do_rinsert(live_ast_ptr act, live_obj_ptr obj)
{
    if( !obj->is_list() ){
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
        live_obj_ptr result;
        if( act->tag == 'c' ){
            switch( act->val.YYint ){
            case '+':
            case '-':
            result = obj_alloc(0);
            break;
            case '/':
            case '*':
            result = obj_alloc(1);
            break;
            default:
            return undefined();
            }
        } else if ( act->tag == 'i' ){
            switch( (act->val.YYsym)->sym_val.YYint ){
            case AND:
            result = obj_alloc(true);
            break;
            case OR:
            case XOR:
            result = obj_alloc(false);
            break;
            default:
            return undefined();
            }
        } else return undefined();
        return(result);
    }

	// If the list has only one element, we return that element.
    auto p = obj->cdr();
    if( !p ){
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
    if( p->is_undef() ){
        obj_unref(obj);
        return(p);
    }
    auto result = obj_alloc(nullptr);
    auto q = obj_alloc(obj->car(), result);
    obj->car()->inc_ref();
    q->cadr(p);
    obj_unref(obj);
    return( execute(act,q) );
}

/// Local function to handle the tedious binary inserting
static live_obj_ptr
do_binsert(live_ast_ptr act, live_obj_ptr obj)
{
    if( !obj->is_list() ){
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
        live_obj_ptr result;
        if( act->tag == 'c' ){
            switch( act->val.YYint ){
            case '+':
            case '-':
            result = obj_alloc(0);
            break;
            case '/':
            case '*':
            result = obj_alloc(1);
            break;
            default:
            return undefined();
            }
        } else if ( act->tag == 'i' ){
            switch( (act->val.YYsym)->sym_val.YYint ){
            case AND:
            result = obj_alloc(true);
            break;
            case OR:
            case XOR:
            result = obj_alloc(false);
            break;
            default:
            return undefined();
            }
        } else return undefined();
        return(result);
    }

    // If the list has only one element, we return that element.
    auto p = obj->cdr();
    if( !p ){
        p = obj->car();
        assert(p);
        p->inc_ref();
        obj_unref(obj);
        auto result = static_cast<live_obj_ptr>(p);
        return(result);
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
    obj_ptr q;
    obj_ptr r;
    int x = 0;
    obj_ptr hd = nullptr;
    auto hdp = &hd;
    for( q = obj; p; p = p->cdr() ){
        if( x ){
            r = obj_alloc(nullptr);
            *hdp = r;
            hdp = r->cdr_addr();
            r->car(q->car());
            q->car()->inc_ref();
            q = q->cdr();
            x = 0;
        } else
            x = 1;
    }
    p = obj_alloc(q->car());
    *hdp = p;
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
    auto new_list = obj_alloc(nullptr);
    p = obj_alloc(do_binsert(act,hd), new_list);
    auto result = do_binsert(act,q);
    p->cadr(result);
    obj_unref(obj);
    return(execute(act,p));
}
