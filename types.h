#ifndef TYPES_H
#define TYPES_H

/*
 * A list of arbitrary objects
 */
struct list {
    struct object
    *car,        /* Head of list */
    *cdr;        /* and Tail */
};

/*
 * An object's structure
 */
struct object {
    uint32_t o_type;        /* Type for selecting */
    uint32_t o_refs;    /* Number of current refs, for GC */
    union {
        int o_int;        /* T_INT, T_BOOL */
        double o_double;        /* T_FLOAT */
        struct list o_list;    /* T_LIST */
    } o_val;
};

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
    struct ast *left;
    struct ast *middle;
    struct ast *right;
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
