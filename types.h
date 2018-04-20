#ifndef TYPES_H
#define TYPES_H

#include <string.h>

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
 * sym_type values
 */

enum symtype {
    SYM_BUILTIN = 1, /* A built-in */
    SYM_DEF = 2, /* User-defined */
    SYM_NEW = 3 /* Never seen before! */
};

/*
 * A symbol table entry for an identifier
 */
struct symtab {
    uint32_t sym_type = 0;
    uint32_t padding = 0;
    YYstype sym_val{};
    sym_ptr sym_next = nullptr;
    char *sym_pname = nullptr;
    symtab(const char *pname, uint32_t type)
    {
        sym_pname = strdup(pname);
        sym_type = type;
    }
    
    symtab(const char *pname)
    : symtab(pname, SYM_NEW)
    {
    }
};

#endif
