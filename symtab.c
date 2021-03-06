/*
 * Yet another symbol tabler
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <string.h>
#include "fpcommon.h"
#include "misc.h"
#include "symtab.h"
#include "yystype.h"
#include "symtab_entry.hpp"
#include "y.tab.h"
#include <array>

using std::array;

static constexpr size_t SYMTABSIZE = 101;

/// Our hash table
static array<sym_ptr, SYMTABSIZE> stab;

/// Generate a hash value for a string
static auto
hash(const char *p)
{
    assert(p);
    assert(strlen(p) > 0);
    size_t s = 0;
    size_t c;

    while( (c = static_cast<size_t>(*p++)) )
        s += c;
    return( s % stab.size() );
}

    /*
     * Given a string, go find the entry.  Allocate an entry if there
     *	was none.
     */
live_sym_ptr
lookup(const char *name)
{
    assert(name);
    assert(strlen(name) > 0);
    auto h = hash(name);
    assert(h > 0);
    assert(h < stab.size());
    sym_ptr p = stab[h];

    // No hash hits, must be a new entry
    if( p == nullptr ){
        auto result = new symtab_entry(name);
        stab[h] = result;
        return( result );
    }

    sym_ptr old = nullptr;
	// Had hits, work way down list
    while( p ){
        if( (p->sym_pname == name))
            return(static_cast<live_sym_ptr>(p));
        old = p;
        p = p->sym_next;
    }

	// No hits, add to end of chain
    auto tail = new symtab_entry(name);
    old->sym_next = tail;
    return( tail );
}

/// Local function to do built-in stuffing
static void
stuff(const char *sym, int val)
{
    assert(sym);
    assert(strlen(sym) > 0);
    auto p = lookup(sym);

    if( p->type() != symtype::SYM_NEW )
        fatal_err("Dup init in stuff()");
    p->type(symtype::SYM_BUILTIN);
    p->sym_val.YYint = val;
}

/// Fill in symbol table with built-ins
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
