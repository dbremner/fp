/*
 * Yet another symbol tabler
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "symtab.h"
#include <string.h>

static const int SYMTABSIZE = 101;

    /*
     * Our hash table
     */
static struct symtab
    *stab[SYMTABSIZE];

    /*
     * Generate a hash value for a string
     */
static int
hash(const char *p)
{
    int s = 0;
    int c;

    while( (c = *p++) ) s += c;
    return( s % SYMTABSIZE );
}

    /*
     * Given a string, go find the entry.  Allocate an entry if there
     *	was none.
     */
sym_ptr
lookup(const char *name)
{
    int h = hash(name);
    sym_ptr p = stab[h];
	sym_ptr old = nullptr;

	/*
	 * No hash hits, must be a new entry
	 */
    if( p == nullptr ){
	return( stab[h] = new symtab(name) );
    }

	/*
	 * Had hits, work way down list
	 */
    while( p ){
	if( strcmp(p->sym_pname,name) == 0 ) return(p);
	old = p;
	p = p->sym_next;
    }

	/*
	 * No hits, add to end of chain
	 */
    return( old->sym_next = new symtab(name) );
}

    /*
     * Local function to do built-in stuffing
     */
static void
stuff(const char *sym, int val)
{
    sym_ptr p = lookup(sym);

    if( p->sym_type != SYM_NEW ) fatal_err("Dup init in stuff()");
    p->sym_type = SYM_BUILTIN;
    p->sym_val.YYint = val;
}

    /*
     * Fill in symbol table with built-ins
     */
void
symtab_init(void){
    stuff( "and", AND );
    stuff( "or", OR );
    stuff( "xor", XOR );
    stuff( "sin", SIN );
    stuff( "cos", COS );
    stuff( "tan", TAN );
    stuff( "asin", ASIN );
    stuff( "acos", ACOS );
    stuff( "atan", ATAN );
    stuff( "log", LOG );
    stuff( "exp", EXP );
    stuff( "mod", MOD );
    stuff( "concat", CONCAT );
    stuff( "last", LAST );
    stuff( "first", FIRST );
    stuff( "tl", TL );
    stuff( "hd", HD );
    stuff( "id", ID );
    stuff( "atom", ATOM );
    stuff( "eq", EQ );
    stuff( "not", NOT );
    stuff( "null", NIL );
    stuff( "reverse", REVERSE );
    stuff( "distl", DISTL );
    stuff( "distr", DISTR );
    stuff( "length", LENGTH );
    stuff( "trans", TRANS );
    stuff( "apndl", APNDL );
    stuff( "apndr", APNDR );
    stuff( "tlr", TLR );
    stuff( "front", FRONT );
    stuff( "rotl", ROTL );
    stuff( "rotr", ROTR );
    stuff( "iota", IOTA );
    stuff( "pair", PAIR );
    stuff( "split", SPLIT );
    stuff( "out", OUT );
    stuff( "while", WHILE );
    stuff( "pick", PICK );
    stuff( "div", DIV );
    stuff( "T", T );
    stuff( "F", F );
}
