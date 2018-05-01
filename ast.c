/*
 * Routines for allocating & freeing AST nodes
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fpcommon.h"
#include "ast.h"
#include "yystype.h"
#include "ast.hpp"
#include "obj.h"

#ifdef MEMSTAT
int ast_out = 0;
static void inc_count() { ast_out++;}
static void dec_count() { ast_out--;}
#else
static void inc_count() {}
static void dec_count() {}
#endif

/// Get a node
live_ast_ptr
ast_alloc(int atag, ast_ptr l, ast_ptr m, ast_ptr r)
{
    inc_count();
    
    auto p = new ast{atag, l, m, r};
    return( p );
}

/// Free a node
static void
ast_free(ast_ptr p)
{
    assert(p);
    dec_count();
    delete p;
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
