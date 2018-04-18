#
# Makefile for fp
#
#	Copyright (c) 1986 by Andy Valencia
#
# Compile-time options
#	-DMEMSTAT to get run-time memory statitistics/checking
#	-DYYDEBUG to get parser tracing
DEFS=
#
# Name your math library here.  On the HP-9000/320, for instance, naming
#	-l881 instead of -lm will use the 68881 coprocessor.
#
MathLibs= -lfpa -lm -lfpa
#
CFLAGS= -O $(DEFS)
OBJS= y.tab.o symtab.o lex.o misc.o ast.o obj.o \
	exec.o charfn.o intrin.o defun.o
fp: $(OBJS)
	cc -o fp $(CFLAGS) $(OBJS) $(MathLibs)
y.tab.h y.tab.c: parse.y fp.h
	yacc -d parse.y
y.tab.o: y.tab.c
	cc $(CFLAGS) -c y.tab.c
lex.o: symtab.h lex.c y.tab.h
symtab.o: symtab.c symtab.h fp.h y.tab.h
ast.o: ast.c fp.h
obj.o: obj.c fp.h
exec.o: exec.c fp.h y.tab.h
charfn.o: charfn.c fp.h y.tab.h
instrinsics.o: instrinsics.c fp.h y.tab.h
defun.o: defun.c symtab.h fp.h y.tab.h
