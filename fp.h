/*
 * Common definitions for FP
 *
 *	Copyright (c) 1986 by Andy Valencia
 */

//enable assertions in all builds
#undef NDEBUG
#include <assert.h>

#include "obj_type.hpp"
#include "types.h"

//ast.c
ast_ptr ast_alloc(int atag);
ast_ptr ast_alloc(int atag, ast_ptr l);
ast_ptr ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r);
void ast_freetree(ast_ptr p);

//charfn.c
obj_ptr do_charfun(ast_ptr act, obj_ptr obj);
obj_ptr eqobj(obj_ptr obj);
obj_type numargs(obj_ptr obj);

//defun.c
void defun(sym_ptr name, ast_ptr def);

//exec.c
obj_ptr execute(ast_ptr  act, obj_ptr obj);

//intrin.c
int listlen(obj_ptr p);
obj_ptr do_intrinsics(sym_ptr act, obj_ptr obj);

//lex.c
void set_prompt(char ch);
int yylex(void);

//misc.c
[[noreturn]] void fatal_err(const char *msg);
void yyerror(const char *msg);

//obj.c
obj_ptr obj_alloc(obj_type);
obj_ptr obj_alloc(int value);
obj_ptr obj_alloc(bool value);
obj_ptr obj_alloc(double value);
///generates the undefined object & returns it
obj_ptr undefined(void);
void obj_prtree(obj_ptr p);
void obj_unref(obj_ptr p);

//YACC runtime
int yyparse(void);
