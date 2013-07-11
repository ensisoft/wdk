
#pragma once

#ifdef LINUX
#  include "X11/types.h"
#elif defined(WINDOWS)
#  include "win32/types.h"
#endif

namespace wdk
{
    typedef int           int_t;
    typedef unsigned int  uint_t;
    typedef unsigned int  bitflag_t;
    typedef unsigned int  ms_t;

    const ms_t NO_TIMEOUT = -1;

} // wdk
