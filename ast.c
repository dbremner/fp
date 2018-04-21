/*
 * Routines for allocating & freeing AST nodes
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"

static ast_ptr ast_list = nullptr;

#ifdef MEMSTAT
int ast_out = 0;
static void inc_count() { ast_out++;}
static void dec_count() { ast_out--;}
#else
static void inc_count() {}
static void dec_count() {}
#endif

/// Get a node
ast_ptr
ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r)
{
    inc_count();
    ast_ptr p = ast_list;
    if(p){
        ast_list = p->left;
        p->init(atag, l, m, r);
        return( p );
    } else {
        p = new ast{};
        p->init(atag, l, m, r);
        return( p );
    }
}

/// Free a node
static void
ast_free(ast_ptr p)
{
    dec_count();
    if( !p ) fatal_err("NULL node in ast_free()");
    p->left = ast_list;
    ast_list = p;
}

/// Free a whole tree
void
ast_freetree(ast_ptr p)
{
    if( !p ) return;
    ast_freetree(p->left);
    ast_freetree(p->right);
    ast_freetree(p->middle);
    if( p->tag == '%' )
	obj_unref( (p->val).YYobj );
    ast_free(p);
}
