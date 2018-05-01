#ifndef OBJECT_HPP
#define OBJECT_HPP

#include "obj_type.hpp"

/// An object's structure
struct object {
private:
    /// Type for selecting
    const obj_type o_type;
public:
    /// Number of current refs, for GC
    unsigned o_refs = 1;
private:
    /// T_INT, T_BOOL
    int o_int = 0;
    /// T_FLOAT
    const double o_double;
    /// Head of list
    obj_ptr car_ = nullptr;
    /// and Tail
    obj_ptr cdr_ = nullptr;
    
public:
    object(obj_type type)
    : object(type, 0, 0.0)
    {
    }
    
private:
    explicit object(obj_type type, int value, double float_val, obj_ptr car_in=nullptr, obj_ptr cdr_in=nullptr)
    : o_type{type},
    o_int{value},
    o_double{float_val}
    {
        car(car_in);
        cdr(cdr_in);
    }
    
public:
    
    explicit object(int value)
    : object(obj_type::T_INT, value, 0.0)
    {
    }
    
    explicit object(bool value)
    : object(obj_type::T_BOOL, value, 0.0)
    {
    }
    
    explicit object(double value)
    : object(obj_type::T_FLOAT, 0, value)
    {
    }
    
    explicit object(obj_ptr car_in)
    : object(obj_type::T_LIST, 0, 0, car_in, nullptr)
    {
    }
    
    explicit object(obj_ptr car_in, obj_ptr cdr_in)
    : object(obj_type::T_LIST, 0, 0, car_in, cdr_in)
    {
    }
    
    ///CAR manipulates the object as a list & gives its first part
    obj_ptr car()
    {
        assert(is_list());
        return car_;
    }

private:
    obj_ptr car() const
    {
        assert(is_list());
        return car_;
    }

public:
    void car(obj_ptr ptr)
    {
        assert(is_list());
        car_ = ptr;
    }
    
    ///CDR is like CAR but gives all but the first
    obj_ptr cdr()
    {
        assert(is_list());
        return cdr_;
    }

private:
    obj_ptr cdr() const
    {
        assert(is_list());
        return cdr_;
    }

public:
    void cdr(obj_ptr ptr)
    {
        assert(is_list());
        cdr_ = ptr;
    }
    
    ///(car (cdr x))
    obj_ptr cadr()
    {
        assert(is_list());
        assert(cdr());
        return cdr()->car();
    }

private:
    ///(car (cdr x))
    obj_ptr cadr() const
    {
        assert(is_list());
        assert(cdr());
        return cdr()->car();
    }

public:
    void cadr(obj_ptr ptr)
    {
        //TODO change annotation
        assert(ptr);
        assert(is_list());
        assert(cdr());
        cdr()->car(ptr);
    }
    
    obj_ptr *cdr_addr()
    {
        return &cdr_;
    }
    
    void inc_ref()
    {
        o_refs++;
    }
    
    void dec_ref()
    {
        o_refs--;
    }
    
    bool is_bool() const
    {
        return type() == obj_type::T_BOOL;
    }
    
    /// returns true if o_type is T_INT or T_FLOAT
    bool is_num() const
    {
        return ( (is_int()) || (is_float()) );
    }
    
    bool is_undef() const
    {
        return type() == obj_type::T_UNDEF;
    }
    
    bool is_int() const
    {
        return type() == obj_type::T_INT;
    }
    
    bool is_float() const
    {
        return type() == obj_type::T_FLOAT;
    }
    
    bool is_list() const
    {
        return type() == obj_type::T_LIST;
    }
    
    /// is_pair()--tell if our argument object is a list of two elements
    bool is_pair() const;
    
    double num_val() const
    {
        assert(is_num());
        return ( is_int() ? \
                (int_val()) : (float_val()) );
    }
    
    double float_val() const
    {
        assert(is_float());
        return o_double;
    }
    
    bool bool_val() const
    {
        return o_int;
    }
    
    void bool_val(bool value)
    {
        o_int = value;
    }
    
    int int_val() const
    {
        return o_int;
    }
    
    obj_type type() const
    {
        return o_type;
    }
    
    /// list_length()--return length of a list
    int list_length() const;
};

#endif
