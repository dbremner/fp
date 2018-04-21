#include <stdbool.h>
#include <stdint.h>

/*
 * Common definitions for FP
 *
 *	Copyright (c) 1986 by Andy Valencia
 */

/// The symbolic names for the different types
enum class obj_type {
    /// Integer
    T_INT = 1,
    /// Floating point
    T_FLOAT = 2,
    /// A LISP-style list
    T_LIST = 3,
    /// The undefined object
    T_UNDEF = 4,
    /// A boolean value
    T_BOOL = 5,
};

#include "types.h"

ast_ptr ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r);
obj_ptr obj_alloc(obj_type);
obj_ptr execute(ast_ptr  act, obj_ptr obj);
obj_ptr invoke(sym_ptr def, obj_ptr obj);
void ast_freetree(ast_ptr p);
[[noreturn]] void fatal_err(const char *msg);
void defun(sym_ptr name, ast_ptr def);
void symtab_init(void);
void obj_prtree(obj_ptr p);
void obj_unref(obj_ptr p);
sym_ptr lookup(const char *name);

///generates the undefined object & returns it
static inline obj_ptr undefined(void)
{
    return(obj_alloc(obj_type::T_UNDEF));
}

///CAR manipulates the object as a list & gives its first part
static inline obj_ptr car_(obj_ptr x)
{
    return (x->o_val).o_list.car;
}

///CDR is like CAR but gives all but the first
static inline obj_ptr cdr_(obj_ptr x)
{
    return ((x)->o_val).o_list.cdr;
}

///(car (cdr x))
static inline obj_ptr cadr_(obj_ptr x)
{
    return car_(cdr_(x));
}

//charfn.c
obj_ptr do_charfun(ast_ptr act, obj_ptr obj);
obj_ptr eqobj(obj_ptr obj);
obj_type numargs(obj_ptr obj);

//intrin.c
int listlen(obj_ptr p);
obj_ptr do_intrinsics(sym_ptr act, obj_ptr obj);

//lex.c
int yylex(void);

//misc.c
void yyerror(const char *msg);

//YACC runtime
int yyparse(void);
