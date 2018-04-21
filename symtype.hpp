#ifndef SYMTYPE_HPP
#define SYMTYPE_HPP

/// sym_type values
enum class symtype {
    /// A built-in
    SYM_BUILTIN = 1,
    /// User-defined
    SYM_DEF = 2,
    /// Never seen before!
    SYM_NEW = 3
};

#endif
