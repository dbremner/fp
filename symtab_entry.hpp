#ifndef SYMTAB_ENTRY_HPP
#define SYMTAB_ENTRY_HPP

#include "symtype.hpp"

#include <string>

/// A symbol table entry for an identifier
struct symtab_entry final {
    symtype sym_type;
    YYstype sym_val{};
    sym_ptr sym_next = nullptr;
    const std::string sym_pname;
    
    symtab_entry(const char *pname)
    :   sym_type{symtype::SYM_NEW},
    sym_pname{pname}
    {
    }
    
    /// Define a function
    void define(live_ast_ptr def);
    
    bool is_defined() const
    {
        return sym_type == symtype::SYM_DEF;
    }
    
    bool is_builtin() const
    {
        return sym_type == symtype::SYM_BUILTIN;
    }
    
    symtype type() const
    {
        return sym_type;
    }
    
    void type(symtype new_type)
    {
        sym_type = new_type;
    }
};

#endif
