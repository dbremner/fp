#include "fpcommon.h"
#include "obj_type.hpp"
#include "object.hpp"

bool object::is_pair() const
{
    if( !is_list() )
        return(false);
    if( car() == nullptr )
        return(false);
    if( cdr() == nullptr )
        return(false);
    if( cdr()->cdr() )
        return(false);
    return(true);
}

/// list_length()--return length of a list
int object::list_length() const
{
    auto p = this;
    int l = 0;
    
    while( p && p->car() ){
        ++l;
        p = p->cdr();
    }
    return(l);
}
