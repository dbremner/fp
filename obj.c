/*
 * obj.c--implement the type "object" and its operators
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <stdio.h>
#include "fpcommon.h"
#include "obj_type.hpp"
#include "obj.h"
#include "object.hpp"

#ifdef MEMSTAT
int obj_out = 0;
static void incobjcount(void) { obj_out++;}
static void decobjcount(void) { obj_out--;}
#else
static void incobjcount(void) {}
static void decobjcount(void) {}
#endif

live_obj_ptr
obj_alloc(int value)
{
    incobjcount();
    return new object{value};
}

live_obj_ptr
obj_alloc(bool value)
{
    incobjcount();
    return new object{value};
}

live_obj_ptr
obj_alloc(double value)
{
    incobjcount();
    return new object{value};
}

live_obj_ptr
obj_alloc(obj_ptr car_, obj_ptr cdr_)
{
    incobjcount();
    return new object{car_, cdr_};
}

live_obj_ptr undefined(void)
{
    incobjcount();
    auto obj = new object{obj_type::T_UNDEF};
    return(obj);
}

/// Free an object
static void
obj_free(obj_ptr p)
{
    assert(p);
    decobjcount();
    delete p;
}

    /*
     * Unreference this pointer, updating objects which it might
     *	reference.
     */
void
obj_unref(obj_ptr p)
{
    if( !p ) return;
    if( --(p->o_refs) ) return;
    switch( p->type() ){
    case obj_type::T_INT:
    case obj_type::T_FLOAT:
    case obj_type::T_UNDEF:
    case obj_type::T_BOOL:
	obj_free(p);
	return;
    case obj_type::T_LIST:
	obj_unref( p->car() );
	obj_unref( p->cdr() );
	obj_free(p);
	return;
    }
}

static char last_close = 0;
void
obj_prtree(obj_ptr p)
{
    if( !p ) return;
    switch( p->type() ){
    case obj_type::T_INT:
	last_close = 0;
	printf("%d ",p->int_val());
    return;
    case obj_type::T_FLOAT:
	last_close = 0;
	printf("%.9g ",p->float_val());
    return;
    case obj_type::T_BOOL:
	last_close = 0;
	printf("%s ",
	    p->bool_val() ? "T" : "F");
    return;
    case obj_type::T_UNDEF:
	last_close = 0;
	printf("? ");
    return;
    case obj_type::T_LIST:
	printf("<");
	last_close = 0;
	if( !p->car() ){
	    printf(">");
	    last_close = 1;
	    return;
	}
	while( p ){
	    obj_prtree( p->car() );
	    p = p->cdr();
	}
	if( !last_close ) putchar('\b');
	printf("> ");
	last_close = 1;
	return;
    }
}
