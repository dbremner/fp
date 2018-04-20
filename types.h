#ifndef TYPES_H
#define TYPES_H

typedef struct ast* ast_ptr;
typedef struct object* obj_ptr;
typedef struct symtab* sym_ptr;

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
    ast_ptr YYast;
    obj_ptr YYobj;
    struct list *YYlist;
    sym_ptr YYsym;
} YYstype;
#define YYSTYPE YYstype

/*
 * An AST
 */
struct ast {
    int tag = 0;
    int padding = 0;
    YYSTYPE val{};
    ast_ptr left = nullptr;
    ast_ptr middle = nullptr;
    ast_ptr right = nullptr;
};

/*
 * A symbol table entry for an identifier
 */
struct symtab {
    uint32_t sym_type;
    uint32_t padding;
    YYstype sym_val;
    sym_ptr sym_next;
    char *sym_pname;
};

#endif
