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
    
    void init(obj_type type)
    {
        o_type = type;
        o_refs = 1;
        o_val.o_list.car = nullptr;
        o_val.o_list.cdr = nullptr;
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
};

#endif
