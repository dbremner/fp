#ifndef TYPES_H
#define TYPES_H

#include <string>

typedef struct ast* ast_ptr;
typedef struct object* obj_ptr;
typedef struct symtab_entry* sym_ptr;

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
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
private:
    int padding = 0;
#pragma clang diagnostic pop
public:
    int tag = 0;
    YYSTYPE val{};
    ast_ptr left = nullptr;
    ast_ptr middle = nullptr;
    ast_ptr right = nullptr;
    
    ast(int tag_, ast_ptr left_, ast_ptr middle_, ast_ptr right_)
    {
        tag = tag_;
        left = left_;
        middle = middle_;
        right = right_;
    }
};

#include "symtype.hpp"

#include "symtab_entry.hpp"

#endif
