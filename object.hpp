#ifndef OBJECT_HPP
#define OBJECT_HPP

/// An object's structure
struct object {
    /// Type for selecting
    obj_type o_type;
    /// Number of current refs, for GC
    unsigned o_refs = 1;
    union {
        /// T_INT, T_BOOL
        int o_int;
        /// T_FLOAT
        double o_double;
        /// T_LIST
        struct list o_list;
    } o_val{};
    
    object(obj_type type)
    : o_type{type}
    {
    }
    
    explicit object(int value)
    : object(obj_type::T_INT)
    {
        o_val.o_int = value;
    }
    
    explicit object(bool value)
    : object(obj_type::T_BOOL)
    {
        o_val.o_int = value;
    }
    
    explicit object(double value)
    : object(obj_type::T_FLOAT)
    {
        o_val.o_double = value;
    }
    
    ///CAR manipulates the object as a list & gives its first part
    obj_ptr car()
    {
        return o_val.o_list.car;
    }
    
    void car(obj_ptr ptr)
    {
        o_val.o_list.car = ptr;
    }
    
    ///CDR is like CAR but gives all but the first
    obj_ptr cdr()
    {
        return o_val.o_list.cdr;
    }
    
    void cdr(obj_ptr ptr)
    {
        o_val.o_list.cdr = ptr;
    }
    
    ///(car (cdr x))
    obj_ptr cadr()
    {
        return cdr()->car();
    }
    
    void inc_ref()
    {
        o_refs++;
    }
    
    void dec_ref()
    {
        o_refs--;
    }
    
    ///ISNUM provides a boolean saying if the named object is a number
    bool is_num() const
    {
        return ( (o_type == obj_type::T_INT) || (o_type == obj_type::T_FLOAT) );
    }
    
    double num_val() const
    {
        return ( (o_type == obj_type::T_INT) ? \
                (o_val.o_int) : (o_val.o_double) );
    }
    
    bool bool_val()
    {
        return o_val.o_int;
    }
    
    void bool_val(bool val)
    {
        o_val.o_int = val;
    }
};

#endif
