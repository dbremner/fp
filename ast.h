#ifndef AST_H
#define AST_H

live_ast_ptr ast_alloc(int atag, ast_ptr l = nullptr, ast_ptr m = nullptr, ast_ptr r = nullptr);
void ast_freetree(ast_ptr p);

#endif
