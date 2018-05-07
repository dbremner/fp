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
    
    symtab_entry(symtab_entry &&other) = default;
    symtab_entry&operator=(symtab_entry &&other) = default;
    
    /// Define a function
    void define(live_ast_ptr def);
    bool is_defined() const;
    bool is_builtin() const;
    symtype type() const;
    void type(symtype new_type);
};

#endif
