#ifndef TYPES_H
#define TYPES_H

/*
 * To alleviate typing in YACC, this type embodies all the
 *    types which "yylval" might receive.
 */
typedef union {
    int YYint;
    double YYdouble;
    ast_ptr YYast;
    obj_ptr YYobj;
    list_ptr YYlist;
    live_sym_ptr YYsym;
} YYstype;
#define YYSTYPE YYstype

#endif
