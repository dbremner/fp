/*
 * Routines for allocating & freeing AST nodes
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "y.tab.h"
#include <stdlib.h>

static struct ast *ast_list = 0;

#ifdef MEMSTAT
int ast_out = 0;
#endif

    /*
     * Get a node
     */
struct ast *
ast_alloc(int atag, struct ast *l, struct ast *m, struct ast *r)
{
    struct ast *p;

#ifdef MEMSTAT
    ast_out++;
#endif
    p = ast_list;
    if(p){
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
ast_free(struct ast *p)
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
ast_freetree(struct ast *p)
{
    if( !p ) return;
    ast_freetree(p->left);
    ast_freetree(p->right);
    ast_freetree(p->middle);
    if( p->tag == '%' )
	obj_unref( (p->val).YYobj );
    ast_free(p);
}
