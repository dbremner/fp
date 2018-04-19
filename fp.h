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

    /*
     * A list of arbitrary objects
     */
struct list {
    struct object
	*car,		/* Head of list */
	*cdr;		/* and Tail */
};

    /*
     * An object's structure
     */
struct object {
    uint32_t o_type;		/* Type for selecting */
    uint32_t o_refs;	/* Number of current refs, for GC */
    union {
        int o_int;		/* T_INT, T_BOOL */
        double o_double;		/* T_FLOAT */
        struct list o_list;	/* T_LIST */
    } o_val;
};

#include "types.h"

struct ast *ast_alloc(int atag, struct ast *l, struct ast *m, struct ast *r);
struct object *obj_alloc(uint32_t);
struct object *execute(struct ast * act, struct object *obj);
struct object *invoke(struct symtab *def, struct object *obj);
void ast_free(struct ast *p);
void ast_freetree(struct ast *p);
noreturn void fatal_err(const char *msg);
void defun(struct symtab *name, struct ast *def);
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

//charfn.c
struct object *do_charfun(struct ast *act, struct object *obj);
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
