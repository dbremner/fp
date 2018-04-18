#ifndef TYPES_H
#define TYPES_H

/*
 * To alleviate typing in YACC, this type embodies all the
 *    types which "yylval" might receive.
 */
typedef union {
    int YYint;
    double YYdouble;
    struct ast *YYast;
    struct object *YYobj;
    struct list *YYlist;
    struct symtab *YYsym;
} YYstype;
#define YYSTYPE YYstype

/*
 * An AST
 */
struct ast {
    int tag;
    int padding;
    YYSTYPE val;
    struct ast *left, *middle, *right;
};

/*
 * A symbol table entry for an identifier
 */
struct symtab {
    uint32_t sym_type;
    uint32_t padding;
    YYstype sym_val;
    struct symtab *sym_next;
    char *sym_pname;
};

#endif
