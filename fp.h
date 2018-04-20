#include <stdbool.h>
#include <stdint.h>
#include <stdnoreturn.h>

/*
 * Common definitions for FP
 *
 *	Copyright (c) 1986 by Andy Valencia
 */

    /*
     * The symbolic names for the different types
     */
#define T_INT 1		/* Integer */
#define T_FLOAT 2	/* Floating point */
#define T_LIST 3	/* A LISP-style list */
#define T_UNDEF 4	/* The undefined object */
#define T_BOOL 5	/* A boolean value */

#include "types.h"

ast_ptr ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r);
struct object *obj_alloc(uint32_t);
struct object *execute(ast_ptr  act, struct object *obj);
struct object *invoke(struct symtab *def, struct object *obj);
void ast_free(ast_ptr p);
void ast_freetree(ast_ptr p);
noreturn void fatal_err(const char *msg);
void defun(struct symtab *name, ast_ptr def);
void symtab_init(void);
void obj_prtree(struct object *p);
void obj_free(struct object *p);
void obj_unref(struct object *p);
struct symtab *lookup(const char *name);

///generates the undefined object & returns it
static inline struct object *undefined(void)
{
    return(obj_alloc(T_UNDEF));
}

///CAR manipulates the object as a list & gives its first part
static inline struct object *car(struct object *x)
{
    return (x->o_val).o_list.car;
}

///CDR is like CAR but gives all but the first
static inline struct object *cdr(struct object *x)
{
    return ((x)->o_val).o_list.cdr;
}

///ISNUM provides a boolean saying if the named object is a number
static inline bool isnum(struct object *x)
{
    return ( (x->o_type == T_INT) || (x->o_type == T_FLOAT) );
}

//charfn.c
struct object *do_charfun(ast_ptr act, struct object *obj);
struct object *eqobj(struct object *obj);
int numargs(struct object *obj);

//intrin.c
int listlen(struct object *p);
struct object *do_intrinsics(struct symtab *act, struct object *obj);

//lex.c
int yylex(void);

//misc.c
void yyerror(const char *msg);

//YACC runtime
int yyparse(void);
