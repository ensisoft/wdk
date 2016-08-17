CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(wdk)

# Only enable release and debug builds
IF(CMAKE_CONFIGURATION_TYPES)
  SET(CMAKE_CONFIGURATION_TYPES Debug Release)
  SET(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}" CACHE STRING
    "Reset the configurations to what we need"
    FORCE)
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Debug)
    MESSAGE("Defaulting to Debug build...")
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_DEBUG_POSTFIX   "d" CACHE STRING "add a postfix, usually d on windows")
SET(CMAKE_RELEASE_POSTFIX ""  CACHE STRING "add a postfix, usually empty on windows")

IF(CMAKE_BUILD_TYPE MATCHES "Release")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_RELEASE_POSTFIX}")
ELSEIF (CMAKE_BUILD_TYPE MATCHES "Debug")
    SET(CMAKE_BUILD_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
ENDIF()


# Solution
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

INCLUDE_DIRECTORIES(BEFORE
    ${CMAKE_CURRENT_LIST_DIR}
)

IF (WIN32)
    ADD_LIBRARY(wdk_system STATIC
        wdk/window_listener.cpp
        wdk/keys.cpp
        wdk/win32/pixmap.cpp
        wdk/win32/system.cpp
        wdk/win32/window.cpp)

    ADD_LIBRARY(wdk_desktop_gl STATIC
        wdk/opengl/WGL/config.cpp
        wdk/opengl/WGL/context.cpp
        wdk/opengl/WGL/surface.cpp)

    # TODO: build the mobile opengl library (using EGL)
    # but for this we need the EGL headers
ELSEIF(UNIX)
    ADD_LIBRARY(wdk_system STATIC
        wdk/window_listener.cpp
        wdk/keys.cpp
        wdk/X11/keysym2ucs.cpp
        wdk/X11/pixmap.cpp
        wdk/X11/system.cpp
        wdk/X11/window.cpp)
    TARGET_LINK_LIBRARIES(wdk_system PUBLIC
        X11 xcb Xau Xdmcp Xxf86vm Xext Xrandr)

    ADD_LIBRARY(wdk_desktop_gl STATIC
        wdk/opengl/GLX/config.cpp
        wdk/opengl/GLX/context.cpp
        wdk/opengl/GLX/surface.cpp)
    TARGET_LINK_LIBRARIES(wdk_desktop_gl PUBLIC
        GL wdk_system)


    # Add this build definition to build the mobile version.
    ADD_LIBRARY(wdk_mobile_gl STATIC
        wdk/opengl/EGL/config.cpp
        wdk/opengl/EGL/context.cpp
        wdk/opengl/EGL/egldisplay.cpp
        wdk/opengl/EGL/surface.cpp)
    TARGET_COMPILE_DEFINITIONS(wdk_mobile_gl PRIVATE "WDK_MOBILE")
    TARGET_LINK_LIBRARIES(wdk_mobile_gl PUBLIC
        GLESv2 EGL wdk_system)

ENDIF()

# build the sample applications

# Open a window and print events to the console.
ADD_EXECUTABLE(SimpleEventSample sample/events.cpp)
TARGET_LINK_LIBRARIES(SimpleEventSample wdk_system)


# Open a window and draw using "big" desktop GL
ADD_EXECUTABLE(DesktopGLSample sample/triangle.cpp)
TARGET_LINK_LIBRARIES(DesktopGLSample
    wdk_system wdk_desktop_gl)

IF(UNIX)
    # Open a window and draw using "mobile" GL
    ADD_EXECUTABLE(MobileGLSample sample/triangle.cpp)
    TARGET_COMPILE_DEFINITIONS(MobileGLSample PRIVATE "SAMPLE_GLES")
    TARGET_LINK_LIBRARIES(MobileGLSample
        wdk_system wdk_mobile_gl)

ENDIF()