/*
 * Common definitions for FP
 *
 *	Copyright (c) 1986 by Andy Valencia
 */

//enable assertions in all builds
#undef NDEBUG
#include <assert.h>

#include "obj_type.hpp"

#include "typedefs.h"

#include "list.h"

#include "types.h"

//ast.c
ast_ptr ast_alloc(int atag);
ast_ptr ast_alloc(int atag, ast_ptr l);
ast_ptr ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r);
void ast_freetree(ast_ptr p);

#include "charfn.h"

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

//YACC runtime
int yyparse(void);
