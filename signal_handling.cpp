#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include "lex.h"
#include "signal_handling.h"

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

void set_signal_handlers()
{
    signal(SIGFPE, badmath);
    signal(SIGINT, intr);
}
