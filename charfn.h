#ifndef CHARFN_H
#define CHARFN_H

#include "pair_type.hpp"

live_obj_ptr do_charfun(live_ast_ptr act, live_obj_ptr obj);
live_obj_ptr eqobj(live_obj_ptr obj);
pair_type pairtype(live_obj_ptr obj);

#endif
