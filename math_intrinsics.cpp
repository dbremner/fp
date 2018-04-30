#include <math.h>
#include "fpcommon.h"
#include "misc.h"
#include "obj.h"
#include "obj_type.hpp"
#include "object.hpp"
#include "y.tab.h"
#include "math_intrinsics.h"

live_obj_ptr
do_math_func(int tag, live_obj_ptr obj)
{
    if( !obj->is_num() ){
        obj_unref(obj);
        return undefined();
    }
    const auto f = obj->num_val();
    double result;
    switch(tag) {
        case SIN: {        // sin() function
            result = sin(f);
            break;
        }
            
        case COS: {        // cos() function
            result = cos(f);
            break;
        }
            
        case TAN: {        // tan() function
            result = tan(f);
            break;
        }
            
        case ASIN: {        // asin() function
            result = asin(f);
            break;
        }
            
        case ACOS: {        // acos() function
            result = acos(f);
            break;
        }
            
        case ATAN: {        // atan() function
            result = atan(f);
            break;
        }
            
        case EXP: {        // exp() function
            result = exp(f);
            break;
        }
            
        case LOG: {        // log() function
            result = log(f);
            break;
        }
        default: {
            fatal_err("Unreachable case in do_trig");
        }
    }
    auto p = obj_alloc(result);
    obj_unref(obj);
    return(p);
}
