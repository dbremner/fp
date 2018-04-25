/*
 * Common definitions for FP
 *
 *	Copyright (c) 1986 by Andy Valencia
 */

//enable assertions in all builds
#undef NDEBUG
#include <assert.h>

//ast.c
ast_ptr ast_alloc(int atag);
ast_ptr ast_alloc(int atag, ast_ptr l);
ast_ptr ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r);
void ast_freetree(ast_ptr p);

//exec.c
obj_ptr execute(ast_ptr act, obj_ptr obj);
