#ifndef OBJECT_INLINES_HPP
#define OBJECT_INLINES_HPP

///CAR manipulates the object as a list & gives its first part
static inline obj_ptr car_(obj_ptr x)
{
    return (x->o_val).o_list.car;
}

#endif
