/*
 * A standard lexical analyzer
 *
 *	Copyright (c) 1986 by Andy Valencia
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "fpcommon.h"
#include "lex.h"
#include "symtab.h"
#include "yystype.h"
#include "symtab_entry.hpp"
#include "y.tab.h"

static char buf[80];
static int donum(char startc);
extern YYSTYPE yylval;

static FILE *cur_in = stdin;
static int nextc(void);

void fp_cmd(void);

static char prompt;

/// How deep can we get?
constexpr size_t MAXNEST = 5;
/// For nested loads
static FILE *fstack[MAXNEST];
static int fpos = 0;

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
    while( (c = nextc()) != EOF )
        if( !isspace(c) ) break;
    ungetc(c,cur_in);
}

/// Lexical analyzer for YACC
int
yylex(void){
    char *p = buf;
    int c;
    int c1;

	// Skip over white space
    skipwhite();
    c = nextc();

	// Return EOF
    if( c == EOF )
        return(c);

	// An "identifier"?
    if( isalpha(c) ){
	    // Assemble a "word" out of the input stream, symbol table it
        *p++ = static_cast<char>(c);
        while( isalnum(c = nextc()) ) *p++ = static_cast<char>(c);
        ungetc(c,cur_in);
        *p = '\0';
        sym_ptr q = lookup(buf);

            // yylval is always set to the symbol table entry
        yylval.YYsym = q;

            // For built-ins, return the token value
        if( q->sym_type == symtype::SYM_BUILTIN ) return( q->sym_val.YYint );

            /*
             * For user-defined (or new),
             *	return "User Defined"--UDEF
             */
        return( UDEF );
    }

	// For numbers, call our number routine.
    if( isdigit(c) )
        return( donum(static_cast<char>(c)) );

	/*
	 * For possible unary operators, see if a digit
	 *	immediately follows.
	 */
    if( (c == '+') || (c == '-') ){
        char c2 = nextc();

        ungetc(c2,cur_in);
        if( isdigit(c2) )
            return( donum(static_cast<char>(c)) );
    }

	/*
	 * For certain C operators, need to look at following char to
	 *	assemble relationals.  Otherwise, just return the char.
	 */
    yylval.YYint = c;
    switch( c ){
        case '<':
            if( (c1 = nextc()) == '=' ) return( yylval.YYint = LE );
            ungetc( c1, cur_in );
            return(c);
        case '>':
            if( (c1 = nextc()) == '=' ) return( yylval.YYint = GE );
            ungetc( c1, cur_in );
            return(c);
        case '~':
            if( (c1 = nextc()) == '=' ) return( yylval.YYint = NE );
            ungetc( c1, cur_in );
            return(c);
        default:
            return(c);
    }
}

static int
donum(char startc)
{
    char isdouble = 0;
    char c, *p = buf;

    *p++ = startc;
    for(;;){
        c = static_cast<char>(nextc());
        if( isdigit(c) ){
            *p++ = c;
            continue;
        }
        if( c == '.' ){
            *p++ = c;
            isdouble = 1;
            continue;
        }
        ungetc( c, cur_in );
        break;
    }
    *p = '\0';
    if( isdouble ){
        sscanf(buf,"%lf",&(yylval.YYdouble));
        return( FLOAT );
    } else {
        sscanf(buf,"%d",&(yylval.YYint));
        return( INT );
    }
}

    /*
     * getchar() function for lexical analyzer.  Adds a prompt if
     *	input is from keyboard, also localizes I/O redirection.
     */
static int
nextc(void){
    int c;
    static int saw_eof = 0;

    do {
        if( cur_in == stdin ){
            if( saw_eof ) {
                return(EOF);
            }
            if( 0 /*!stdin->_cnt*/ ) {//TODO
                putchar(prompt);
            }
        }
        c = fgetc(cur_in);
        if( c == '#' ){
            bool newline = false;
            while( (c = fgetc(cur_in)) != EOF ) {
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
            if( cur_in != stdin ){
                fclose(cur_in);
                cur_in = fstack[--fpos];
                continue;
            } else {
                saw_eof++;
            }
        }
    } while(0);
    return(c);
}

[[noreturn]] static void
quit(void)
{
    printf("\nDone\n");
    exit( 0 );
}

static void
help(void)
{
    printf("Commands are:\n");
    printf(" quit - leave FP\n");
    printf(" help - this message\n");
    printf(" load - redirect input from a file\n");
#ifdef YYDEBUG
    printf(" yydebug - toggle parser tracing\n");
#endif
    return;
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
fp_cmd(void){
    char cmd[80], *p = cmd, arg[80];
    int c;

	// Assemble a word, the command
    skipwhite();
    if( (c = nextc()) == EOF )
        return;
    *p++ = static_cast<char>(c);
    while( (c = nextc()) != EOF )
        if( isalpha(c) )
            *p++ = static_cast<char>(c);
        else
            break;
    *p = '\0';

	// Process the command
    if( strcmp(cmd,"load") == 0 ){	// Load command

            // Get next word, the file to load
        skipwhite();
        p = arg;
        while( (c = nextc()) != EOF )
            if( isspace(c) )
                break;
            else
                *p++ = static_cast<char>(c);
        *p = '\0';

            // Can we push down any more?
        if( fpos == MAXNEST-1 ){
            printf(")load'ed files nested too deep\n");
            return;
        }

            // Try and open the file
        FILE *newf;
        if( (newf = fopen(arg,"r")) == 0 ){
            perror(arg);
            return;
        }

            // Pushdown the current file, make this one it.
        fstack[fpos++] = cur_in;
        cur_in = newf;
        return;
    }

    if( strcmp(cmd,"quit") == 0 ){	// Leave
        quit();
    }
    if( strcmp(cmd,"help") == 0 ){	// Give help
        help();
        return;
    }
#ifdef YYDEBUG
    if( strcmp(cmd,"yydebug") == 0 ){	// Toggle parser trace
        flipyydebug();
        return;
    }
#endif
    printf("Unknown command '%s'\n",cmd);
}
