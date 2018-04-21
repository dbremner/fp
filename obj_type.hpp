#ifndef OBJ_TYPE_HPP
#define OBJ_TYPE_HPP

/// The symbolic names for the different types
enum class obj_type {
    /// Integer
    T_INT = 1,
    /// Floating point
    T_FLOAT = 2,
    /// A LISP-style list
    T_LIST = 3,
    /// The undefined object
    T_UNDEF = 4,
    /// A boolean value
    T_BOOL = 5
};

#endif
