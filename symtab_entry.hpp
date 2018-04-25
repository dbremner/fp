#ifndef SYMTAB_ENTRY_HPP
#define SYMTAB_ENTRY_HPP

#include "symtype.hpp"

#include <string>

/// A symbol table entry for an identifier
struct symtab_entry {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
private:
    unsigned padding = 0;
#pragma clang diagnostic pop
public:
    symtype sym_type;
    YYstype sym_val{};
    sym_ptr sym_next = nullptr;
    const std::string sym_pname;
    
    symtab_entry(const char *pname)
    :   sym_type{symtype::SYM_NEW},
    sym_pname{pname}
    {
    }
};

#endif
