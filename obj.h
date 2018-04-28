#ifndef OBJ_H
#define OBJ_H

//obj.c
/// constructs a T_INT
live_obj_ptr obj_alloc(int value);
/// constructs a T_BOOL
live_obj_ptr obj_alloc(bool value);
/// constructs a T_FLOAT
live_obj_ptr obj_alloc(double value);
/// constructs a T_LIST
live_obj_ptr obj_alloc(obj_ptr car_, obj_ptr cdr_ = nullptr);
///generates the undefined object & returns it
live_obj_ptr undefined(void);
void obj_prtree(obj_ptr p);
void obj_unref(obj_ptr p);

#endif
