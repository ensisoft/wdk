
#pragma once

#include <windows.h>

namespace wdk
{
    typedef HWND   native_window_t;
    typedef HANDLE native_handle_t;
    typedef MSG    native_event_t;
    typedef HDC    native_display_t;
    typedef HDC    native_surface_t;
    typedef size_t native_vmode_t;

    const HWND   NULL_WINDOW  = NULL;
    const HANDLE NULL_HANDLE  = NULL;
    const HDC    NULL_SURFACE = NULL;
    const size_t DEFAULT_VIDEO_MODE = -1;

} // wdk


