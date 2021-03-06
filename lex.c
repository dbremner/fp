/*
 * A standard lexical analyzer
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "fpcommon.h"
#include "lex.h"
#include "symtab.h"
#include "yystype.h"
#include "symtab_entry.hpp"
#include "y.tab.h"
#include "file_stack.hpp"

static constexpr size_t LINELENGTH = 80;

static char buf[LINELENGTH];
static int donum(char startc);
extern YYSTYPE yylval;

//TODO inject this?
#pragma clang diagnostic ignored "-Wglobal-constructors"
static file_stack stack;

static int nextc(void);

static char prompt;

void set_prompt(char ch)
{
    assert(isascii(ch));
    prompt = ch;
}

/// Skip leading white space in current input stream
static void
skipwhite(){
    int c;

	// Skip leading blank space
    while( (c = nextc()) != EOF ) {
        if( !isspace(c) ) break;
    }
    stack.ungetc(c);
}

/// Lexical analyzer for YACC
/// TODO consider allowing _ in identifiers
int
yylex(void)
{
    // Skip over white space
    skipwhite();
    int c = nextc();

    // Return EOF
    if( c == EOF ) {
        return(c);
    }

    // An "identifier"?
    if( isalpha(c) ){
        size_t index = 0;
	    // Assemble a "word" out of the input stream, symbol table it
        buf[index++] = static_cast<char>(c);
        while( isalnum(c = nextc()) ) {
            buf[index++] = static_cast<char>(c);
        }
        stack.ungetc(c);
        buf[index] = '\0';
        auto q = lookup(buf);

            // yylval is always set to the symbol table entry
        yylval.YYsym = q;

            // For built-ins, return the token value
        if( q->is_builtin() ) {
            return( q->sym_val.YYint );
        }

            /*
             * For user-defined (or new),
             *	return "User Defined"--UDEF
             */
        return( UDEF );
    }

	// For numbers, call our number routine.
    if( isdigit(c) ) {
        return( donum(static_cast<char>(c)) );
    }

	/*
	 * For possible unary operators, see if a digit
	 *	immediately follows.
	 */
    if( (c == '+') || (c == '-') ){
        int c2 = nextc();

        stack.ungetc(c2);
        if( isdigit(c2) ) {
            return( donum(static_cast<char>(c)) );
        }
    }

	/*
	 * For certain C operators, need to look at following char to
	 *	assemble relationals.  Otherwise, just return the char.
	 */
    yylval.YYint = c;
    int c1;
    switch( c ){
        case '<': {
            c1 = nextc();
            if( c1 == '=' ) {
                return( yylval.YYint = LE );
            }
            stack.ungetc(c1);
            return(c);
        }
        case '>': {
            c1 = nextc();
            if( c1 == '=' ) {
                return( yylval.YYint = GE );
            }
            stack.ungetc(c1);
            return(c);
        }
        case '~': {
            c1 = nextc();
            if( c1 == '=' ) {
                return( yylval.YYint = NE );
            }
            stack.ungetc(c1);
            return(c);
        }
        default: {
            return(c);
        }
    }
}

static int
donum(char startc)
{
    char isdouble = 0;
    size_t index = 0;
    buf[index++] = startc;
    for(;;){
        const auto c = static_cast<char>(nextc());
        if( isdigit(c) ){
            buf[index++] = c;
            continue;
        }
        if( c == '.' ){
            buf[index++] = c;
            isdouble = 1;
            continue;
        }
        stack.ungetc(c);
        break;
    }
    buf[index] = '\0';
    if( isdouble ){
        sscanf(buf,"%lf",&(yylval.YYdouble));
        return( FLOAT );
    } else {
        sscanf(buf,"%d",&(yylval.YYint));
        return( INT );
    }
}

    /**
     * getchar() function for lexical analyzer.  Adds a prompt if
     *	input is from keyboard, also localizes I/O redirection.
     * TODO calls to this function should check for EOF
     */
static int
nextc(void){
    int c;
    static int saw_eof = 0;

    do {
        if( stack.is_stdin() ){
            if( saw_eof ) {
                return(EOF);
            }
            
            if( isatty(fileno(stdin))) {//TODO
                putchar(prompt);
            }
        }
        c = stack.fgetc();
        if( c == '#' ){
            bool newline = false;
            while( (c = stack.fgetc()) != EOF ) {
                if( c == '\n' ) {
                    newline = true;
                    break;
                }
            }
            if (newline) {
                continue;
            }
        }
        // Pop up a level of indirection on EOF
        if( c == EOF ){
            if( !stack.is_stdin() ){
                stack.pop();
                continue;
            } else {
                saw_eof++;
            }
        }
    } while(0);
    return(c);
}

static void
load()
{
    char arg[LINELENGTH]{};
    // Get next word, the file to load
    skipwhite();
    size_t index = 0;
    int c;
    while( (c = nextc()) != EOF ) {
        if( isspace(c) ) {
            break;
        }
        else {
            arg[index++] = static_cast<char>(c);
        }
    }
    arg[index] = '\0';
    
    // Can we push down any more?
    if( stack.is_full() ){
        printf(")load'ed files nested too deep\n");
        return;
    }
    
    // Try and open the file
    FILE * _Nullable newf = fopen(arg,"r");
    if( newf == nullptr ){
        perror(arg);
        return;
    }
    
    stack.push(newf);
    return;
}

static void help();
[[noreturn]] static void quit();
static void load();

struct command
{
    const char * _Nonnull const name;
    void (* _Nonnull func)(void);
    const char * _Nonnull const description;
};

static const command commands[] =
{
    {"load", load, " load - redirect input from a file\n"},
    {"quit", quit, " quit - leave FP\n"},
    {"help", help, " help - this message\n"},
#ifdef YYDEBUG
    {"yydebug", flipyydebug, " yydebug - toggle parser tracing\n"},
#endif
};

[[noreturn]] static void
quit(void)
{
    printf("\nDone\n");
    exit(EXIT_SUCCESS);
}

static void
help(void)
{
    printf("Commands are:\n");
    for(const auto &it: commands) {
        printf("%s", it.description);
    }
}

#ifdef YYDEBUG
static void
flipyydebug(void)
{
    extern int yydebug;
    
    yydebug = !yydebug;
    return;
}
#endif


    /*
     * Command processor.  The reason it's here is that we play with
     *	I/O redirection.  Shrug.
     */
void
fp_cmd(void)
{
    // Assemble a word, the command
    skipwhite();
    int c = nextc();
    if( c == EOF ) {
        return;
    }
    char cmd[LINELENGTH]{};
    size_t index = 0;
    cmd[index++] = static_cast<char>(c);
    while( (c = nextc()) != EOF ) {
        if( isalpha(c) ) {
            cmd[index++] = static_cast<char>(c);
        }
        else {
            break;
        }
    }
    cmd[index] = '\0';
    
    for(const auto &iter : commands)
    {
        const auto name = iter.name;
        if (strcmp(cmd, name) == 0)
        {
            const auto func = iter.func;
            func();
            return;
        }
    }
    printf("Unknown command '%s'\n",cmd);
}
