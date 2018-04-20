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
    obj_ptr car;        /* Head of list */
    obj_ptr cdr;        /* and Tail */
};

/*
 * An object's structure
 */
struct object {
    obj_type o_type;        /* Type for selecting */
    uint32_t o_refs = 1;    /* Number of current refs, for GC */
    union {
        int o_int;        /* T_INT, T_BOOL */
        double o_double;        /* T_FLOAT */
        struct list o_list;    /* T_LIST */
    } o_val{};
    
    object(obj_type type)
    {
        o_type = type;
    }
    
    ///CAR manipulates the object as a list & gives its first part
    obj_ptr car()
    {
        return (this->o_val).o_list.car;
    }
    
    ///CDR is like CAR but gives all but the first
    obj_ptr cdr()
    {
        return ((this)->o_val).o_list.cdr;
    }
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

enum class symtype {
    SYM_BUILTIN = 1, /* A built-in */
    SYM_DEF = 2, /* User-defined */
    SYM_NEW = 3 /* Never seen before! */
};

/*
 * A symbol table entry for an identifier
 */
struct symtab {
    symtype sym_type;
    uint32_t padding = 0;
    YYstype sym_val{};
    sym_ptr sym_next = nullptr;
    char *sym_pname = nullptr;
    
    symtab(const char *pname)
    {
        sym_pname = strdup(pname);
        sym_type = symtype::SYM_NEW;
    }
};

#endif
