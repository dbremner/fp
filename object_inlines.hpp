#ifndef OBJECT_INLINES_HPP
#define OBJECT_INLINES_HPP

///generates the undefined object & returns it
static inline obj_ptr undefined(void)
{
    return(obj_alloc(obj_type::T_UNDEF));
}

///CAR manipulates the object as a list & gives its first part
static inline obj_ptr car_(obj_ptr x)
{
    return (x->o_val).o_list.car;
}

///(car (cdr x))
static inline obj_ptr cadr_(obj_ptr x)
{
    return car_(x->cdr());
}

#endif
