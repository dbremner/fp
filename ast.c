/*
 * Routines for allocating & freeing AST nodes
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"

static struct ast *ast_list = 0;

#ifdef MEMSTAT
int ast_out = 0;
#endif

    /*
     * Get a node
     */
struct ast *
ast_alloc(atag,l,m,r)
    int atag;
    struct ast *l, *m, *r;
{
    register struct ast *p;

#ifdef MEMSTAT
    ast_out++;
#endif
    if( p = ast_list ){
	ast_list = p->left;
    } else {
	p = (struct ast *)malloc(sizeof(struct ast));
    }
    if( p == 0 ) fatal_err("Out of mem in ast_alloc()");
    p->tag = atag;
    p->left = l;
    p->middle = m;
    p->right = r;
    return( p );
}

    /*
     * Free a node
     */
void
ast_free(p)
    register struct ast *p;
{
#ifdef MEMSTAT
    ast_out--;
#endif
    if( !p ) fatal_err("NULL node in ast_free()");
    p->left = ast_list;
    ast_list = p;
}

    /*
     * Free a whole tree
     */
void
ast_freetree(p)
    struct ast *p;
{
    if( !p ) return;
    ast_freetree(p->left);
    ast_freetree(p->right);
    ast_freetree(p->middle);
    if( p->tag == '%' )
	obj_unref( (p->val).YYobj );
    ast_free(p);
}
