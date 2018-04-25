#ifndef OBJ_H
#define OBJ_H

//obj.c
obj_ptr obj_alloc(obj_type);
/// constructs a T_INT
obj_ptr obj_alloc(int value);
/// constructs a T_BOOL
obj_ptr obj_alloc(bool value);
/// constructs a T_FLOAT
obj_ptr obj_alloc(double value);
/// constructs a T_LIST
obj_ptr obj_alloc(obj_ptr car_, obj_ptr cdr_ = nullptr);
///generates the undefined object & returns it
obj_ptr undefined(void);
void obj_prtree(obj_ptr p);
void obj_unref(obj_ptr p);

#endif
