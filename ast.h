#ifndef AST_H
#define AST_H

live_ast_ptr ast_alloc(int atag);
live_ast_ptr ast_alloc(int atag, ast_ptr l);
live_ast_ptr ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r);
void ast_freetree(ast_ptr p);

#endif
