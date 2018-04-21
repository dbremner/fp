%{
   /*
    * FP syntax for YACC
    *
    *	Copyright (c) 1986 by Andy Valencia
    */
#include "fp.h"
#include <stdio.h>

#define NULLAST ((ast_ptr)0)
static char had_undef = 0;
void fp_cmd(void);

#ifdef MEMSTAT
extern int obj_out, ast_out;
#endif

//all of these are triggered by bison's generated code
#pragma clang diagnostic ignored "-Wunreachable-code"
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wreserved-id-macro"
#pragma clang diagnostic ignored "-Wmissing-variable-declarations"
#pragma clang diagnostic ignored "-Wold-style-cast"
%}

%start go

%token INT FLOAT T F ID UDEF AND OR XOR NE GT LT GE LE
%token SIN COS TAN ASIN ACOS ATAN LOG EXP MOD CONCAT LAST FIRST PICK
%token TL HD ATOM NOT EQ NIL REVERSE DISTL DISTR LENGTH DIV
%token TRANS APNDL APNDR TLR ROTL ROTR IOTA PAIR SPLIT OUT
%token FRONT

%token WHILE
%token '[' ']'
%right '@'
%right '%' '!' '&' '|'

%%
go	:	go fpInput
	|	go error
		    { yyclearin; }
	|	Empty
	;

fpInput
	:	fnDef
		    {
#ifdef MEMSTAT
    if( obj_out || ast_out ){
	printf("%d objects and %d AST nodes used in definition\n",
	  obj_out,ast_out);
	obj_out = ast_out = 0;
    }
#endif
		    }
	|	application
		    {
#ifdef MEMSTAT
    if( obj_out || ast_out ){
	printf("%d objects lost, %d AST nodes lost\n",obj_out,ast_out);
	obj_out = ast_out = 0;
    }
#endif
		    }
	|	')'
		    { fp_cmd(); }
	;

fnDef	:	'{'
		    { set_prompt('>'); }
		name funForm
		'}'
		    {
			defun($3.YYsym,$4.YYast);
			set_prompt('\t');
		    }
	;

application
	:	    { set_prompt('-'); }
	    funForm ':' object
		    {
			obj_ptr p = execute($2.YYast,$4.YYobj);

			obj_prtree(p);
			printf("\n");
			obj_unref(p);
			ast_freetree($2.YYast);
			set_prompt('\t');
		    }
	;

name	:	UDEF
	;

object	:	object2
		    {
			    /*
			     * If the luser, say, makes <1 2 <3 ?>>,
			     *	we need to flatten it to ?.
			     */
			if( had_undef ){
			    obj_unref($1.YYobj);
			    $$.YYobj = obj_alloc(obj_type::T_UNDEF);
			    had_undef = 0;
			}
		    }
	;
object2	:	atom
	|	fpSequence
	|	'?'
		    {
			$$.YYobj = obj_alloc(obj_type::T_UNDEF);
			had_undef = 1;
		    }
	;

fpSequence
	:	'<' object2 OptComma SeqBody '>'
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_LIST);
			(p->o_val).o_list.car = $2.YYobj;
			(p->o_val).o_list.cdr = $4.YYobj;
		    }
	;
SeqBody	:	Empty
		    {
			$$.YYobj = 0;
		    }
	|	object2 OptComma SeqBody
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_LIST);
			(p->o_val).o_list.car = $1.YYobj;
			(p->o_val).o_list.cdr = $3.YYobj;
		    }
	;

atom	:	T
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_BOOL);
			(p->o_val).o_int = 1;
		    }
	|	F
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_BOOL);
			(p->o_val).o_int = 0;
		    }
	|	'<' '>'
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_LIST);
			(p->o_val).o_list.car =
			    (p->o_val).o_list.cdr = 0;
		    }
	|	INT
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_INT);
			(p->o_val).o_int = $1.YYint;
		    }
	|	FLOAT
		    {
			obj_ptr p = 
			    $$.YYobj = obj_alloc(obj_type::T_FLOAT);
			(p->o_val).o_double = $1.YYdouble;
		    }
	;

funForm	:	simpFn
	|	composition
	|	construction
	|	conditional
	|	constantFn
	|	insertion
	|	alpha
	|	While
	|	'(' funForm ')'
		    {
			$$ = $2;
		    }
	;

simpFn	:	IdFns
		    {
			$$.YYast = ast_alloc('i', NULLAST, NULLAST, NULLAST);
			(($$.YYast)->val).YYsym = $1.YYsym;
		    }
	|	INT
		    {
			$$.YYast = ast_alloc('S', NULLAST, NULLAST, NULLAST);
			(($$.YYast)->val).YYint = $1.YYint;
		    }
	|	binaryFn
		    {
			$$.YYast = ast_alloc('c', NULLAST, NULLAST, NULLAST);
			(($$.YYast)->val).YYint = $1.YYint;
		    }
	|	name
		    {
			$$.YYast = ast_alloc('U', NULLAST, NULLAST, NULLAST);
			(($$.YYast)->val).YYsym = $1.YYsym;
		    }
	;

IdFns
	:	TL
	|	DIV
	|	HD
	|	EQ
	|	ATOM
	|	PICK
	|	NOT
	|	NIL
	|	REVERSE
	|	DISTL
	|	DISTR
	|	LENGTH
	|	TRANS
	|	APNDL
	|	APNDR
	|	TLR
	|	FRONT
	|	ROTL
	|	ROTR
	|	IOTA
	|	PAIR
	|	SPLIT
	|	CONCAT
	|	LAST
	|	FIRST
	|	OUT
	|	SIN
	|	COS
	|	TAN
	|	ASIN
	|	ACOS
	|	ATAN
	|	LOG
	|	EXP
	|	MOD
	|	OR
	|	AND
	|	XOR
	|	ID
	;

binaryFn
	:	'<'
	|	'>'
	|	'='
	|	GE
	|	LE
	|	NE
	|	'+'
	|	'-'
	|	'*'
	|	'/'
	;

composition
	:	funForm '@' funForm
		    {
			$$.YYast = ast_alloc('@',$1.YYast,NULLAST,$3.YYast);
		    }
	;

construction
	:	'[' formList ']'
		    {
			$$.YYast = ast_alloc('[',$2.YYast,NULLAST,NULLAST);
		    }
	;

formList
	:	funForm
		    {
			$$.YYast = ast_alloc('[',$1.YYast,NULLAST,NULLAST);
		    }
	|	funForm OptComma formList
		    {
			$$.YYast = ast_alloc('[',$1.YYast,NULLAST,$3.YYast);
		    }
	;

conditional
	:	'(' funForm '-' '>' funForm ';' funForm ')'
		    {
			$$.YYast = ast_alloc('>',$2.YYast,$5.YYast,$7.YYast);
		    }
	;

constantFn
	:	'%' object
		    {
			$$.YYast = ast_alloc('%',NULLAST,NULLAST,NULLAST);
			(($$.YYast)->val).YYobj = $2.YYobj;
		    }
	;

insertion
	:	'!' funForm
		    {
			$$.YYast = ast_alloc('!',$2.YYast,NULLAST,NULLAST);
		    }
	|	'|' funForm
		    {
			$$.YYast = ast_alloc('|',$2.YYast,NULLAST,NULLAST);
		    }
	;

alpha	:	'&' funForm
		    {
			$$.YYast = ast_alloc('&',$2.YYast,NULLAST,NULLAST);
		    }
	;

While	:	'(' WHILE funForm funForm ')'
		    {
			$$.YYast = ast_alloc('W',$3.YYast,NULLAST,$4.YYast);
		    }
	;

Empty	:	/* Nothing */
	;

OptComma			/* Optional comma */
	:	Empty
	|	','
	;
%%
