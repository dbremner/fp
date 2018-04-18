#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Common definitions for FP
 *
 *	Copyright (c) 1986 by Andy Valencia
 */

    /*
     * Aliases for unsigned quantities.  Not really any reason, just
     *	couldn't resist wasting a bit...
     */
typedef unsigned char uchar;
typedef unsigned long int uint;

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
    uchar o_type;		/* Type for selecting */
    uint o_refs;		/* Number of current refs, for GC */
    union {
	int o_int;		/* T_INT, T_BOOL */
	double o_double;		/* T_FLOAT */
	struct list o_list;	/* T_LIST */
    } o_val;
};

extern struct ast *ast_alloc();
extern struct object *obj_alloc(), *execute(), *invoke();
extern void ast_free(), ast_freetree(), fatal_err(), defun(),
	symtab_init(), obj_free(), obj_unref(), obj_prtree();
extern struct symtab *lookup();


    /*
     * To alleviate typing in YACC, this type embodies all the
     *	types which "yylval" might receive.
     */
typedef union {
    int YYint;
    double YYdouble;
    struct ast *YYast;
    struct object *YYobj;
    struct list *YYlist;
    struct symtab *YYsym;
} YYstype;
#define YYSTYPE YYstype

    /*
     * An AST
     */
struct ast {
    int tag;
    YYSTYPE val;
    struct ast *left, *middle, *right;
};

    /*
     * A symbol table entry for an identifier
     */
struct symtab {
    uchar sym_type;
    YYstype sym_val;
    struct symtab *sym_next;
    char *sym_pname;
};

