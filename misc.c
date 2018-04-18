/*
 * Miscellaneous functions
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include "fp.h"
#include <setjmp.h>
#include <signal.h>

extern char prompt;

static jmp_buf restart;

void
fatal_err(char *msg)
{
    printf("Fatal error: %s\n",msg);
    exit( 1 );
}

void
yyerror(char *msg)
{
    printf("yyerror() reports '%s'\n",msg);
    prompt = '\t';
}

    /*
     * Floating exception handler
     */
static void
badmath(){
    printf("Floating exception\n");
    prompt = '\t';
    signal(SIGFPE, badmath);
    longjmp(restart,1);
}

    /*
     * User interrupt handler
     */
static void
intr(){
    printf("Interrupt\n");
    prompt = '\t';
    signal(SIGINT, intr);
    longjmp(restart,1);
}

main() {
    symtab_init();
    prompt = '\t';

    signal(SIGFPE, badmath);
    signal(SIGINT, intr);

    if( setjmp(restart) == 0 )
	printf("FP v0.0\n");
    else
	printf("FP restarted\n");
    yyparse();
    printf("\nFP done\n");
    exit( 0 );
    /*NOTREACHED*/
}
