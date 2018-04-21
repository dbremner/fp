/*
 * obj.c--implement the type "object" and its operators
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include <stdio.h>

static obj_ptr free_objs = nullptr;

#ifdef MEMSTAT
int obj_out = 0;
static void incobjcount(void) { obj_out++;}
static void decobjcount(void) { obj_out--;}
#else
static void incobjcount(void) {}
static void decobjcount(void) {}
#endif

/// Allocate an object
obj_ptr
obj_alloc(obj_type ty)
{
    incobjcount();
	// Have a free one on the list
    obj_ptr p = free_objs;
    if(p){
        free_objs = (p->o_val).o_list.car;
        p->init(ty);
    } else {
        return ::new object{ty};
    }
    return(p);
}

/// Free an object
static void
obj_free(obj_ptr p)
{
    decobjcount();
    if( !p ) fatal_err("Null object to obj_free()");
    (p->o_val).o_list.car = free_objs;
    free_objs = p;
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
    switch( p->o_type ){
    case obj_type::T_INT:
    case obj_type::T_FLOAT:
    case obj_type::T_UNDEF:
    case obj_type::T_BOOL:
	obj_free(p);
	return;
    case obj_type::T_LIST:
	obj_unref( (p->o_val).o_list.car );
	obj_unref( (p->o_val).o_list.cdr );
	obj_free(p);
	return;
    default:
	fatal_err("Unknown type in obj_unref()");
    }
}

static char last_close = 0;
void
obj_prtree(obj_ptr p)
{
    if( !p ) return;
    switch( p->o_type ){
    case obj_type::T_INT:
	last_close = 0;
	printf("%d ",(p->o_val).o_int); return;
    case obj_type::T_FLOAT:
	last_close = 0;
	printf("%.9g ",(p->o_val).o_double); return;
    case obj_type::T_BOOL:
	last_close = 0;
	printf("%s ",
	    (p->o_val).o_int ? "T" : "F"); return;
    case obj_type::T_UNDEF:
	last_close = 0;
	printf("? "); return;
    case obj_type::T_LIST:
	printf("<");
	last_close = 0;
	if( !p->o_val.o_list.car ){
	    printf(">");
	    last_close = 1;
	    return;
	}
	while( p ){
	    obj_prtree( (p->o_val).o_list.car );
	    p = (p->o_val).o_list.cdr;
	}
	if( !last_close ) putchar('\b');
	printf("> ");
	last_close = 1;
	return;
    }
}
