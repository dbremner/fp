#ifndef TYPES_H
#define TYPES_H

/*
 * To alleviate typing in YACC, this type embodies all the
 *    types which "yylval" might receive.
 */
typedef union {
    int YYint;
    double YYdouble;
    live_ast_ptr YYast;
    live_obj_ptr YYobj;
    live_sym_ptr YYsym;
} YYstype;
#define YYSTYPE YYstype

#endif
