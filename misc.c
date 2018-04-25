/*
 * Miscellaneous functions
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fpcommon.h"
#include "lex.h"
#include "misc.h"

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
