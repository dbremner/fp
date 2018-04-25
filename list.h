#ifndef LIST_H
#define LIST_H

/// A list of arbitrary objects
struct list {
    /// Head of list
    obj_ptr car;
    /// and Tail
    obj_ptr cdr;
};

#endif
