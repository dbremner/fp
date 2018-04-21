#ifndef TYPES_H
#define TYPES_H

#include <string>

typedef struct ast* ast_ptr;
typedef struct object* obj_ptr;
typedef struct symtab* sym_ptr;

/// A list of arbitrary objects
struct list {
    /// Head of list
    obj_ptr car;
    /// and Tail
    obj_ptr cdr;
};

#include "object.hpp"

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

/// An AST
struct ast {
    int tag = 0;
    int padding = 0;
    YYSTYPE val{};
    ast_ptr left = nullptr;
    ast_ptr middle = nullptr;
    ast_ptr right = nullptr;
};

/// sym_type values
enum class symtype {
    /// A built-in
    SYM_BUILTIN = 1,
    /// User-defined
    SYM_DEF = 2,
    /// Never seen before!
    SYM_NEW = 3
};

/// A symbol table entry for an identifier
struct symtab {
    symtype sym_type;
    uint32_t padding = 0;
    YYstype sym_val{};
    sym_ptr sym_next = nullptr;
    std::string sym_pname;
    
    symtab(const char *pname)
    :   sym_pname{pname},
        sym_type{symtype::SYM_NEW}
    {
    }
};

#endif
