/*
 * obj.c--implement the type "object" and its operators
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"

static struct object *free_objs = 0;

#ifdef MEMSTAT
int obj_out = 0;
#endif

    /*
     * Allocate an object
     */
struct object *
obj_alloc(uint32_t ty)
{
    struct object *p;

#ifdef MEMSTAT
    obj_out++;
#endif
	/*
	 * Have a free one on the list
	 */
    p = free_objs;
    if(p){
	free_objs = (p->o_val).o_list.car;
    } else if( (p = (struct object *)malloc(sizeof(struct object))) == 0 )
	fatal_err("out of memory in obj_alloc()");
    p->o_refs = 1;
    if( (p->o_type = ty) == T_LIST )
	p->o_val.o_list.car = p->o_val.o_list.cdr = 0;
    return(p);
}

    /*
     * Free an object
     */
void
obj_free(struct object *p)
{
#ifdef MEMSTAT
    obj_out--;
#endif
    if( !p ) fatal_err("Null object to obj_free()");
    (p->o_val).o_list.car = free_objs;
    free_objs = p;
}

    /*
     * Unreference this pointer, updating objects which it might
     *	reference.
     */
void
obj_unref(struct object *p)
{
    if( !p ) return;
    if( --(p->o_refs) ) return;
    switch( p->o_type ){
    case T_INT:
    case T_FLOAT:
    case T_UNDEF:
    case T_BOOL:
	obj_free(p);
	return;
    case T_LIST:
	obj_unref( (p->o_val).o_list.car );
	obj_unref( (p->o_val).o_list.cdr );
	obj_free(p);
	return;
    default:
	fatal_err("Unknown type in obj_unref()");
    }
    /*NOTREACHED*/
}

static char last_close = 0;
void
obj_prtree(struct object *p)
{
    if( !p ) return;
    switch( p->o_type ){
    case T_INT:
	last_close = 0;
	printf("%d ",(p->o_val).o_int); return;
    case T_FLOAT:
	last_close = 0;
	printf("%.9g ",(p->o_val).o_double); return;
    case T_BOOL:
	last_close = 0;
	printf("%s ",
	    (p->o_val).o_int ? "T" : "F"); return;
    case T_UNDEF:
	last_close = 0;
	printf("? "); return;
    case T_LIST:
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
    /*NOTREACHED*/
}
