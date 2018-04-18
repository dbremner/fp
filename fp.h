#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

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

// fwd dcls
struct ast;
struct symtab;
struct object;

extern struct ast *ast_alloc(int atag, struct ast *l, struct ast *m, struct ast *r);
extern struct object *obj_alloc(uint32_t);
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
extern struct symtab *lookup(const char *name);

//charfn.c
struct object *eqobj(struct object *obj);

//intrin.c
int listlen(struct object *p);

//lex.c
int yylex(void);

//misc.c
void yyerror(const char *msg);

//YACC runtime
int yyparse(void);

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
    int padding;
    YYSTYPE val;
    struct ast *left, *middle, *right;
};

    /*
     * A symbol table entry for an identifier
     */
struct symtab {
    uint32_t sym_type;
    uint32_t padding;
    YYstype sym_val;
    struct symtab *sym_next;
    char *sym_pname;
};

