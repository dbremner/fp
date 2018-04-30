#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include "fpcommon.h"
#include "lex.h"
#include "signal_handling.h"
#include "symtab.h"

jmp_buf restart;

//YACC runtime
int yyparse(void);

int
main(void)
{
    lex_init(stdin);
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
