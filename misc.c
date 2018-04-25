/*
 * Miscellaneous functions
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include "symtab.h"
#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static jmp_buf restart;

[[noreturn]] void
fatal_err(const char *msg)
{
    assert(msg);
    assert(strlen(msg) > 0);
    printf("Fatal error: %s\n",msg);
    exit( 1 );
}

void
yyerror(const char *msg)
{
    assert(msg);
    assert(strlen(msg) > 0);
    printf("yyerror() reports '%s'\n",msg);
    set_prompt('\t');
}

extern "C" [[noreturn]] void badmath(int ignored);

/// Floating exception handler
extern "C"
[[noreturn]] void
badmath(int /*ignored*/){
    printf("Floating exception\n");
    set_prompt('\t');
    signal(SIGFPE, badmath);
    longjmp(restart,1);
}

extern "C" [[noreturn]] void intr(int ignored);

/// User interrupt handler
extern "C"
[[noreturn]] void
intr(int /*ignored*/){
    printf("Interrupt\n");
    set_prompt('\t');
    signal(SIGINT, intr);
    longjmp(restart,1);
}

int
main(void) {
    symtab_init();
    set_prompt('\t');

    signal(SIGFPE, badmath);
    signal(SIGINT, intr);

    if( setjmp(restart) == 0 )
	printf("FP v0.0\n");
    else
	printf("FP restarted\n");
    yyparse();
    printf("\nFP done\n");
    exit( 0 );
}
