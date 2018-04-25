/*
 * Miscellaneous functions
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fpcommon.h"
#include "lex.h"
#include "misc.h"
#include "signal_handling.h"
#include "symtab.h"

jmp_buf restart;

[[noreturn]] void
fatal_err(const char *msg)
{
    assert(msg);
    assert(strlen(msg) > 0);
    printf("Fatal error: %s\n",msg);
    exit(EXIT_FAILURE);
}

void
yyerror(const char *msg)
{
    assert(msg);
    assert(strlen(msg) > 0);
    printf("yyerror() reports '%s'\n",msg);
    set_prompt('\t');
}

//YACC runtime
int yyparse(void);

int
main(void) {
    symtab_init();
    set_prompt('\t');

    set_signal_handlers();

    if( setjmp(restart) == 0 )
	printf("FP v0.0\n");
    else
	printf("FP restarted\n");
    yyparse();
    printf("\nFP done\n");
    exit(EXIT_SUCCESS);
}
